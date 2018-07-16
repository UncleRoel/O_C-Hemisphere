// Skewed LFO:
//   Encoder: LFO rate or LFO skew
//   Button: Select rate or skew
//   Out 1: LFO
//   Out 2: Clock
//   Digital 1: Reset

#define HEM_LFO_HIGH 40000
#define HEM_LFO_LOW 800
#define HEM_LFO_MAX_VALUE 120

class SkewedLFO : public HemisphereApplet {
public:

    const char* applet_name() {
        return "SkewedLFO";
    }

    void Start() {
        rate = 61;
        skew = 61;
        selected = 0;
        cycle_tick = 0;
    }

    void Controller() {
        rate_mod = get_modification_with_input(0);
        skew_mod = get_modification_with_input(1);

        cycle_tick++;

        // Handle reset trigger
        if (Clock(0)) cycle_tick = 0;

        // Handle LFO output
        if (cycle_tick >= TicksAtRate()) {
            cycle_tick = 0;
            ClockOut(1);
        }
        int cv = AmplitudeAtPosition(cycle_tick, HEMISPHERE_MAX_CV) - (HEMISPHERE_MAX_CV / 2); // subtract for bipolar
        Out(0, cv);
    }

    void View() {
        gfxHeader(applet_name());

        DrawSkewedWaveform();
        DrawRateIndicator();
        DrawWaveformPosition();
    }

    void ScreensaverView() {
        DrawSkewedWaveform();
        DrawRateIndicator();
        DrawWaveformPosition();
    }

    void OnButtonPress() {
        selected = 1 - selected;
    }

    void OnEncoderMove(int direction) {
        if (selected == 0) {
            rate = constrain(rate += direction, 0, HEM_LFO_MAX_VALUE);
        } else {
            skew = constrain(skew += direction, 0, HEM_LFO_MAX_VALUE);
        }
    }

    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0, 8}, skew);
        Pack(data, PackLocation {8, 8}, rate);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        skew = Unpack(data, PackLocation {0,8});
        rate = Unpack(data, PackLocation {8,8});
    }

protected:
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "1=Reset";
        help[HEMISPHERE_HELP_CVS] = "Mod 1=Rate 2=Skew";
        help[HEMISPHERE_HELP_OUTS] = "A=CV B=Clock";
        help[HEMISPHERE_HELP_ENCODER] = "Rate/Skew";
    }

private:
    int rate; // LFO rate between 0 and HEM_LFO_MAX_VALUE, where 0 is slowest
    int skew; // LFO skew, where 0 is saw, HEM_LFO_MAX_VALUE is ramp, and 31 is triangle
    int selected; // Whether knob is editing rate (0) or skew (1)
    int cycle_tick; // The current tick number within the cycle
    int rate_mod; // Modification of rate from CV 1
    int skew_mod; // Modification of skew from CV 2

    void DrawRateIndicator() {
        gfxFrame(1, 15, 62, 6);
        int x = Proportion(rate, HEM_LFO_MAX_VALUE, 62);
        gfxLine(x, 15, x, 20);
        if (selected == 0) gfxRect(1, 16, x, 4);
    }

    void DrawSkewedWaveform() {
        int x = Proportion(skew, HEM_LFO_MAX_VALUE, 62);
        gfxLine(0, 62, x, 33, selected == 0);
        gfxLine(x, 33, 62, 62, selected == 0);

        // Draw zero-crossing line
        gfxDottedLine(0, 48, 63, 48, 5);
    }

    void DrawWaveformPosition() {
        int height = AmplitudeAtPosition(cycle_tick, 30);
        int x = Proportion(cycle_tick, TicksAtRate(), 62);
        gfxLine(x, 63, x, 63 - height);
    }

    int AmplitudeAtPosition(int position, int max_amplitude) {
        int amplitude = 0;
        int effective_skew = constrain(skew + skew_mod, 0, HEM_LFO_MAX_VALUE);
        int ticks_at_rate = TicksAtRate();
        int fall_point = Proportion(effective_skew, HEM_LFO_MAX_VALUE, ticks_at_rate);
        if (position < fall_point) {
            // Rise portion
            amplitude = Proportion(position, fall_point, max_amplitude);
        } else {
            // Fall portion
            amplitude = Proportion(ticks_at_rate - position, ticks_at_rate - fall_point, max_amplitude);
        }

        return amplitude;
    }

    int TicksAtRate() {
        int effective_rate = constrain(rate + rate_mod, 0, HEM_LFO_MAX_VALUE);
        int inv_rate = HEM_LFO_MAX_VALUE - effective_rate;
        int range = HEM_LFO_HIGH - HEM_LFO_LOW;
        int ticks_at_rate = Proportion(inv_rate, HEM_LFO_MAX_VALUE, range) + HEM_LFO_LOW;
        return ticks_at_rate;
    }

    int get_modification_with_input(int in) {
        int mod = 0;
        mod = Proportion(DetentedIn(in), HEMISPHERE_MAX_CV, HEM_LFO_MAX_VALUE / 2);
        return mod;
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to SkewedLFO,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
SkewedLFO SkewedLFO_instance[2];

void SkewedLFO_Start(int hemisphere) {
    SkewedLFO_instance[hemisphere].BaseStart(hemisphere);
}

void SkewedLFO_Controller(int hemisphere, bool forwarding) {
    SkewedLFO_instance[hemisphere].BaseController(forwarding);
}

void SkewedLFO_View(int hemisphere) {
    SkewedLFO_instance[hemisphere].BaseView();
}

void SkewedLFO_Screensaver(int hemisphere) {
    SkewedLFO_instance[hemisphere].BaseScreensaverView();
}

void SkewedLFO_OnButtonPress(int hemisphere) {
    SkewedLFO_instance[hemisphere].OnButtonPress();
}

void SkewedLFO_OnEncoderMove(int hemisphere, int direction) {
    SkewedLFO_instance[hemisphere].OnEncoderMove(direction);
}

void SkewedLFO_ToggleHelpScreen(int hemisphere) {
    SkewedLFO_instance[hemisphere].HelpScreen();
}

uint32_t SkewedLFO_OnDataRequest(int hemisphere) {
    return SkewedLFO_instance[hemisphere].OnDataRequest();
}

void SkewedLFO_OnDataReceive(int hemisphere, uint32_t data) {
    SkewedLFO_instance[hemisphere].OnDataReceive(data);
}
