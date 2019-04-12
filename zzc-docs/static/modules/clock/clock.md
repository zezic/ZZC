<!---
start: affixing
affixed: blueprint
blueprint: clock-blueprint.svg
preview: clock.svg
-->

# Clock

<p align='center' class='md-only'>
  <img src='clock.svg'>
</p>

> The serious source of tick-tack for your virtual rack

Clock is a clock source module. It supports...

Donec tristique porta maximus. Ut aliquet, nulla ac volutpat iaculis, lectus nulla congue risus, pretium euismod est enim eu nulla. Fusce sollicitudin erat id nunc accumsan gravida. Fusce tempus pulvinar magna nec aliquam. Integer nec faucibus eros. Aliquam nec consectetur purus, at laoreet nulla. Donec faucibus, erat id egestas dictum, nibh erat gravida leo, vitae sagittis neque nunc sed ante. Aliquam ac suscipit ante. In at vulputate ipsum, dignissim accumsan felis. Vestibulum eleifend risus vel sem hendrerit malesuada nec finibus libero. Mauris tincidunt consectetur enim non pharetra. Aliquam elementum pretium velit non faucibus. Suspendisse quis sagittis quam. Cras viverra, felis sit amet dignissim laoreet, justo massa rutrum libero, et ultricies eros nulla a nulla.

<img align='right' src='clock-blueprint.svg' class='md-only'>

<!---
start: legend
-->

### Inputs

* <!---
  x: 10
  y: 52
  slug: vbps
  type: labeled-socket
  -->
  <a name="inputs-vbps" href='#inputs-vbps'>V/BPS</a> - Volts per beat per second. Input affects the base BPM value. For example, adding 1V to it will raise the base BPM by 1 beat per second (60 BPM). Negative values can make transport go reverse.

* <!---
  x: 10
  y: 145
  slug: run
  type: simple-socket
  -->
  [RUN](#inputs-run) - Phasellus quam felis, posuere eu consequat quis, tempor quis odio. Proin cursus vitae tortor ut sodales.
  > Note, that when plugged vestibulum volutpat felis porta, lacinia justo a, laoreet ex.

* <!---
  x: 114
  y: 145
  slug: rst
  type: simple-socket
  -->
  [RST](#inputs-rst) - External reset input

### Outputs

* <!---
  x: 114
  y: 320
  slug: vspb
  type: labeled-socket
  -->
  [RST](#outputs-vspb) - Volts per second per beat output
  > Kek kek

<!---
end: legend
-->
<!---
end: affixing
-->
