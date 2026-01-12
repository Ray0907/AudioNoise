//
// TS9 Tube Screamer style overdrive
//
// Characteristics:
// - Soft clipping (diode-like)
// - Mid frequency boost (~720Hz)
// - Mix of clean and driven signal
//
struct {
	struct biquad mid_boost;
	struct biquad low_cut;
	float drive;
	float tone;
	float level;
} ts9;

void ts9_init(float pot1, float pot2, float pot3, float pot4)
{
	// pot1: drive (gain before clipping)
	// pot2: tone (high frequency blend)
	// pot3: level (output volume)
	// pot4: unused

	ts9.drive = 1 + pot1 * 30;  // 1x to 31x gain
	ts9.tone = pot2;
	ts9.level = pot3;

	// Mid boost around 720Hz, Q=0.7 for smooth bump
	biquad_bpf_peak(&ts9.mid_boost, 720, 0.7);

	// Low cut to tighten bass, ~720Hz highpass
	biquad_hpf(&ts9.low_cut, 200, 0.7);

	fprintf(stderr, "ts9:");
	fprintf(stderr, " drive=%g", ts9.drive);
	fprintf(stderr, " tone=%g", ts9.tone);
	fprintf(stderr, " level=%g\n", ts9.level);
}

// Soft clipping function (approximates diode clipping)
static inline float ts9_clip(float x)
{
	// tanh-style soft clipping
	if (x > 1.5f) return 1.0f;
	if (x < -1.5f) return -1.0f;
	return x * (1.5f - x * x / 3.0f) / 1.5f;
}

float ts9_step(float in)
{
	float out;

	// Apply drive
	out = in * ts9.drive;

	// Soft clipping
	out = ts9_clip(out);

	// Mid boost (TS9 signature sound)
	out = out + biquad_step(&ts9.mid_boost, out) * 0.5f;

	// Tone control: blend between low-cut and full signal
	float bright = biquad_step(&ts9.low_cut, out);
	out = out * (1 - ts9.tone) + bright * ts9.tone;

	// Output level
	out = out * ts9.level;

	// Mix with clean signal (TS9 keeps some clean)
	return limit_value(out * 0.7f + in * 0.3f);
}
