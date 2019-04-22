<!---
start: affixing
affixed: blueprint
blueprint: clock-blueprint.svg
preview: clock.svg
-->

# Clock

<p align='center' class='md-only'>
  <img src='clock.svg'/>
</p>

> Clock & Phase Generator

Clock - is a pulse generator which is used mainly as a source of information about transport state in modular system. This module provides several unusual features which are differs it from another LFOs or Clocks:

* Beat phase input and output (0V - 10V)
* Synchronization with external clock
* Reverse support
* Two subgenerators of 2x a 4x frequency of main generator with adjustable swing
* V/BPS input/output, V/SPB output

## History

This module was developed because of my demand in information about inter-beat state of transport in VCV Rack. In another words, I wanted to know precisely at which position my transport goes between two beats at the moment. For that I decided to use saw waveform to represent beat progress with its value. By analyzing such continuous signal another modules are able to detect speed of transport and exact position between beat pulses. Later I found out that it's something similar to [Phasor](https://en.wikipedia.org/wiki/Phasor) approach. You can read about its practical usage in DSP world at [Audulus documentation](http://docs.audulus.com/nodes/#phasor).

## Phase

Especially in Clock module only single component is used to represent the phasor. That's because it's easier to pass it to another modules with only single cable (it was before Rack 1.0 was released). To understand it better, please take a look at the next graph:

<img align='middle' src='zzc-phasor.svg'/>

Here **Pulse** - is a usual impulse-like signal signalizing about start of each new beat when **Phase** - is a signal which represents the progress of current beat, in another words - the phase of a beat. It differs from classic phasor by it's range which is 10V instead of Pi. 10V range was taken to be stronger against noise and to simplify analysis and processing of that signal. Of course, it's still hard to use such signal in real-world hardware systems because of non-ideal voltages, cable characteristics, DAC/ADC nuances and other factors, but in ideally perfect digital system it works as intended.

### Speed Changes

When speed (or frequency) of main generator is changed, the phase is immedatially renders it without waiting for the next beat impulse. For example, on the next graph you can see how it looks when transport slowly slowing down to 0 speed:

<img align='middle' src='zzc-phasor-slowdown.svg'/>

And this how it looks when transport stops between beats, changes its direction and goes backward:

<img align='middle' src='zzc-phasor-reverse.svg'/>

## Swing

Clock module provides two additional generators of subimpuleses. First generator outputs pulse of frequency which is 2 times higher that base frequency (x2), second generator outputs pulses with a frequency which is 4 times higher that base (x4). And those generators are interesting because it's possible to change the *swing factor* of each generator. When the *swing factor* is increased every 2nd pulse of the generator will be shifted towrd the next coming pulse and vice versa it will be shifted earlier if *swing factor* is decreased from its default 50% value. Fox example look at the next graph which shows how every 2nd pulse position is affected by *swing factor*:

<img align='middle' src='zzc-clock-swing.svg'/>

The default value for neutral and straight rhythm is 50% (which is middle knob position).

Another interesting fact is the dependency of second (x4) subgenerator on position of pulses of first (x2) subgenerator. It means that when *swing factor* of first (x2) subgenerator is changed, second (x4) subgenerator is placing its impulses in different positions than before, actually it always splits periods of first subgenerator by its own *swing factor*:

<img align='middle' src='zzc-clock-swing-dependency.svg'/>

By adding the power of CV control of swing factors we get an interesting tool which allows to create complex rhythmical patterns.
> Keep in mind that smooth CV modulations of *swing factor* can produce not always wanted *stutter* effect. It happens because of immediate nature of swing CV controls in that module, so its possible for them to constantly place the position of next sub-beat in the way of phase of the main generator which produces the new subimpulses again and again when phase goes forward.

## External Tempo Sources

Clock module support number of synchronization modes.

* External Pulse Source (CLK)
* External Phase Source (PHASE)
* External Source of Clock and Phase (CLK + PHASE)
* External Source of Volts per Beat per Second (V/BPS)

Actual mode is choosen based on the connected inputs. You can see which signals are used for the current synchronization mode by looking at small indicators placed next to the corresponding inputs.

### Synchronization: External Pulse Source (CLK)

When this mode is activated the module is waits for at least 2 pulses on the [CLK input](#inputs-clock). After receiving of second pulse it calculates the tempo by measuring a time interval between first and second pulse. After first successfull tempo detection it continues to constantly track tempo changes. In this mode the phase of a generator can become shifted relative to the pulse source, so it should be resetted manually or by sending a pulse to the [RESET input](#inputs-reset).

### Synchronization: External Phase Source (PHASE)

In the External Phase Source the [PHASE input](#inputs-phase) is used almost directly. It begin to work right after the second sample of mode activity. Ideally, that signal should be a perfect digital saw waveform in range from 0V to 10V, but you can play with its shape to achieve non-straight rhythm of subgenerators.

### Synchronization: External Source of Clock and Phase (CLK + PHASE)

This mode is works almost the same as the previous one. Instead of analyzing abrupt phase changes (when saw ends at its corner and goes down) it uses external clock pulse to determine the beat moment.

### Synchronization: External Source of Volts per Beat per Second (V/BPS)

Better to say that this mode is not a synchronization mode, but a tempo modulation mode.

> #### What is V/BPS?
> Let's make this abbreviation clear. "V/BPS" means "volts per beat per second". It's a bit confusing, but it hides pretty simple meaning - it's a number of beats in second. For example, when tempo is 120BPM we got 120 beats per minute, which equals 2 beats per second. So, when your clock is going at 120BPM you can expect positive 2V from [V/BPS output](#outputs-vbps). And when you applying some voltage to the [V/BPS input](#inputs-vbps) its added to the base BPM value of a main module generator.

So, when you know what V/BPS means, you can use necessary voltage to adjust base BPM setting of a module. To use [V/BPS input](#inputs-vbps) as an absolute value the [BPM knob](#controls-bpm) should be turned down to zero.
Note that negative V/BPS values are also supported, they are turns generator to go backwards and output reversed phase and reversed subimpulse pattern.

## Reverse

Reverse is activated by [REV button](#controls-reverse). Keep in mind that this button does not work when module is synchronized to the external phase.
When main generator is reversed, phase output is changed from raising to falling saw waveform. Also, rhythmical pattern of subgenerators with all its swing parameters becomes reversed and V/BPS values becomes negative. But value on [V/SPB output](#outputs-vspb) (not V/BPS!) does not change its polarity, read the next section to know why.

## V/SPB

V/SPB - is a value representing the time duration of one beat in seconds, it means "volts per second per beat". When tempo is slowing, intervals between beats are increasing in time. V/BPS is displaying the length of those intervals, it can be used in such tempo-dependent effects as Delay, for example. Output value is clamped to 10 volts, it means that 6 BPM is the lowest tempo value for which this output has correct value (60 seconds / 10 seconds = 6 beats).

<img align='right' src='clock-blueprint.svg' class='md-only'/>

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
  y: 52
  slug: vbps
  type: labeled-socket
  -->
  <a name="inputs-vbps" href='#inputs-vbps'>V/BPS</a> - volts per beat per second. Voltage on this output affects the base tempo of module, each volt adds another 60 BPM. Negative values are also supperted and are able to make transport go back.

* <!---
  x: 10
  y: 145
  slug: run
  type: simple-socket
  -->
  <a name="inputs-run" href='#inputs-run'>Run</a> - start/pause (toggle).

* <!---
  x: 114
  y: 145
  slug: reset
  type: simple-socket
  -->
  <a name="inputs-reset" href='#inputs-reset'>Reset</a> - phase reset. Resets phase to zero. Does not work when external phase source is connected.
  > Common way to patch this is to connect [Run output](#outputs-run) to [Reset input](#inputs-reset). Such connection makes module to reset its phase automatically on every pause or start.

* <!---
  x: 10
  y: 224
  slug: swing-x2
  type: simple-socket
  -->
  <a name="inputs-swing-x2" href='#inputs-swing-x2'>x2 Clock Swing</a> - *swing factor* of x2 subimpulse generator. Range is -5V to +5V.

* <!---
  x: 114
  y: 224
  slug: swing-x4
  type: simple-socket
  -->
  <a name="inputs-swing-x4" href='#inputs-swing-x4'>x4 Clock Swing</a> - *swing factor* of x4 subimpulse generator. Range is -5V to +5V.
  > Voltage range of this input and previous input is scaled to the remaining range depending on current position of [Swing Factor](#controls-swing-x4)

* <!---
  x: 45
  y: 224
  slug: clock
  type: labeled-socket
  -->
  <a name="inputs-clock" href='#inputs-clock'>Clock</a> - external clock source.

* <!---
  x: 79
  y: 224
  slug: phase
  type: labeled-socket
  -->
  <a name="inputs-phase" href='#inputs-phase'>Phase</a> - external phase source. Range is from 0V to +10V.

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
  y: 272
  slug: clock-x2
  type: labeled-socket
  -->
  <a name="outputs-clock-x2" href='#outputs-clock-x2'>Clock x2</a> - pulse output of x2 subgenerator.

* <!---
  x: 45
  y: 272
  slug: clock
  type: labeled-socket
  -->
  <a name="outputs-clock" href='#outputs-clock'>Clock</a> - pulse output (clock) of main generator.

* <!---
  x: 79
  y: 272
  slug: phase
  type: labeled-socket
  -->
  <a name="outputs-phase" href='#outputs-phase'>Phase</a> - phase of main generator. Range is from 0V to +10V.

* <!---
  x: 114
  y: 272
  slug: clock-x4
  type: labeled-socket
  -->
  <a name="outputs-clock-x4" href='#outputs-clock-x4'>Clock x4</a> - pulse output of x4 subgenerator.

* <!---
  x: 10
  y: 320
  slug: vbps
  type: labeled-socket
  -->
  <a name="outputs-vbps" href='#outputs-vbps'>V/BPS</a> - volts per beat per second (1V = 1BPS = 60BPM). Negative output means reverse.

* <!---
  x: 45
  y: 320
  slug: run
  type: labeled-socket
  -->
  <a name="outputs-run" href='#outputs-run'>Run</a> - run/pause pulse.

* <!---
  x: 79
  y: 320
  slug: reset
  type: labeled-socket
  -->
  <a name="outputs-reset" href='#outputs-reset'>Reset</a> - phase reset pulse.

* <!---
  x: 114
  y: 320
  slug: vspb
  type: labeled-socket
  -->
  <a name="outputs-vspb" href='#outputs-vspb'>V/SPB</a> - volts per second per beat (1V = 1SPB), equals to the duration of one beat. Typically used as a time value for time-based effects such as Delay.

<!---
end: legend-group
-->

<!---
start: legend-group
slug: controls
-->

### Controls

* <!---
  x: 116
  y: 53
  slug: reverse
  type: labeled-led-switch
  -->
  <a name="controls-reverse" href='#controls-reverse'>Reverse</a> - reverse toggle. Also acts as a reverse indicator. Does not work when external phase is connected.

* <!---
  x: 41
  y: 82
  slug: bpm
  type: big-knob
  -->
  <a name="controls-bpm" href='#controls-bpm'>BPM</a> - BPM of a main generator. Range is from 0 to 240BPM, can be modulated by value on [V/BPS input](#inputs-vbps).

* <!---
  x: 47
  y: 168
  slug: run
  type: labeled-led-switch
  -->
  <a name="controls-run" href='#controls-run'>Run</a> - start/pause toggle.

* <!---
  x: 81
  y: 168
  slug: reset
  type: labeled-led-switch
  -->
  <a name="controls-reset" href='#controls-reset'>Reset</a> - phase reset. Does not work when external phase is used.

* <!---
  x: 13
  y: 186
  slug: swing-x2
  type: knob-27
  -->
  <a name="controls-swing-x2" href='#controls-swing-x2'>Swing x2</a> - *swing factor* of x2 subgenerator.

* <!---
  x: 109
  y: 186
  slug: swing-x4
  type: knob-27
  -->
  <a name="controls-swing-x4" href='#controls-swing-x4'>Swing x4</a> - *swing factor* of x4 subgenerator.
  > Both *swing factor* knobs have marks for values of 25%, 33%, 50%, 66% and 75%.

<!---
end: legend-group
-->

<!---
start: legend-group
slug: indicators
-->

### Indicators

* <!---
  x: 46
  y: 40
  slug: bpm
  type: bpm-display
  -->
  <a name="indicators-bpm" href='#indicators-bpm'>BPM</a> - current tempo of main generator. When external clock or phase source is connected displayed value can jump between fractional parts which is caused by aliasing.

* <!---
  x: 71
  y: 66
  slug: phase
  type: medium-led
  -->
  <a name="indicators-phase" href='#indicators-phase'>Phase</a> - phase indicator. Lights up at phase start and fades out as it comes to the end.

<!---
end: legend-group
-->

<!---
end: legend
-->
<!---
end: affixing
-->
