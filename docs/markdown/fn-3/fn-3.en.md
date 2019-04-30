<!---
start: affixing
affixed: blueprint
blueprint: fn-3-blueprint.svg
preview: fn-3.svg
-->

# FN-3

<p align='center' class='md-only'>
  <img src='fn-3.svg'/>
</p>

> Function Generator

FN-3 - is a simple function generator. It is driven by incoming phase (signal from 0V to 10V) and supports 3 modes - sine, triangle and square wave.

## Sine

Sine mode is a bit different from calling *sin* function over input directly. When input is at 0V it gives result of *sin(x)* with *x = -1/2 Pi*, in other words, it picks lowest point from *sin* function. Then, as input value increases to 10V the *x* value is get increaset to *3/2 Pi*. Such function remapping gives the full period of *sin()* function which starts and ends at zero and contains a single hump.

Such remapping is done to make PWM parameter more usable in sine mode. In sine mode it sets ratio between first and second half-periods of phase. Second reasond for such remapping is better usability with bipolar signals and usual envelopes.

## Triangle

Triangle mode is similar to sine mode, but with straight lines on rising and falling half-periods. What makes it special is the PWM behavior, which allow to morph the triangle waveform to sawtooth by stretching on of its half-periods.

## Square

There is no any special things about the square mode, it is pretty straightforward. One thing to note is that PWM in this mode allow to turn output to constant low-value or constant high-value on its minimum and maximum.

## Development History

At its initial idea this module was created to allow creating long envelopes and LFO's from phase information. For example, you can pick a [Clock module](./clock) phase output, stretch it with [Divider](./divider) to, let's say, 4 bars and convert with FN-3 to long envelope or LFO of desirible form which is perfectly synced to main clock.

Such way is not the only way you can use FN-3, you can transform another LFO signals with it, another envelopes and even audio-rate signals.

<img align='right' src='fn-3-blueprint.svg' class='md-only'/>

## Main Panel

<!---
start: legend
-->

<!---
start: legend-group
slug: inputs
-->

### Inputs

* <!---
  x: 10
  y: 93
  slug: pwm
  type: simple-socket
  -->
  <a name="inputs-pwm" href='#inputs-pwm'>PWM</a> - Pulse Width (Modulation). When [PWM knob](#controls-pwm) is in the middle the useful range of this input is from -5V to +5V. Values over this range are also supported when [PWM knob](#controls-pwm) is not at its central position.

* <!---
  x: 10
  y: 194
  slug: shift
  type: labeled-socket
  -->
  <a name="inputs-shift" href='#inputs-shift'>Shift</a> - phase shift. Range is not limited, each 5V (positive or negative) shifts phase by 360 degrees.

* <!---
  x: 10
  y: 275
  slug: phase
  type: labeled-socket
  -->
  <a name="inputs-phase" href='#inputs-phase'>Phase</a> - incoming phase, this is a main input. Sawtooth waveform in range from 0V to 10V gives expected waveform output, but values not in range (negative) are also supported, so, bipolar signals are also should work.

<!---
end: legend-group
-->

<!---
start: legend-group
slug: outputs
-->

### Outputs

* <!---
  x: 10
  y: 320
  slug: wave
  type: labeled-socket
  -->
  <a name="inputs-wave" href='#inputs-wave'>Wave</a> - resulting waveform. Range depends on [polarity mode](#controls-polarity).

<!---
end: legend-group
-->

<!---
start: legend-group
slug: controls
-->

### Controls

* <!---
  x: 8
  y: 58
  slug: pwm
  type: knob-27
  -->
  <a name="controls-pwm" href='#controls-pwm'>PWM</a> - Pulse Width (Modulation).

* <!---
  x: 8
  y: 126
  slug: mode
  type: display-switch
  -->
  <a name="controls-mode" href='#controls-mode'>Mode</a> - generator mode - SIN, TRI or SQR.

* <!---
  x: 8
  y: 152
  slug: polarity
  type: display-switch
  -->
  <a name="controls-polarity" href='#controls-polarity'>Polarity</a> - UNI (unipolar) or BI (bipolar). When unipolar mode is selected output goes in range from 0V to 10V and with bipolar it works in range from -5V to +5V.

* <!---
  x: 10
  y: 229
  slug: shift
  type: knob-25
  -->
  <a name="controls-shift" href='#controls-shift'>Shift</a> - phase shift. Range of knob is from -360 to +360 degrees.

<!---
end: legend-group
-->

<!---
start: legend-group
slug: indicators
-->

### Indicators

* <!---
  x: 8
  y: 126
  slug: wave
  type: fn-3-display
  -->
  <a name="indicators-wave" href='#indicators-wave'>Waveform</a> - preview of current mode on phase range from 0V to 10V.

<!---
end: legend-group
-->

<!---
end: legend
-->

<!---
end: affixing
-->
