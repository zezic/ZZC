<!---
start: affixing
affixed: blueprint
blueprint: divider-blueprint.svg
preview: divider.svg
-->

# Divider

<p align='center' class='md-only'>
  <img src='divider.svg'/>
</p>

> Делитель фазы

Divider - это модуль, преобразующий частоту пилообразного сигнала в диапазоне от 0V до 10V. С помощью такого сигнала выражается фаза транспорта, которую генерирует модуль [Clock](./clock).
Чем-то Divider похож на делители клока (Clock Divider). Помимо обычного преобразования частоты фазы он умеет придавать ей свинг, это будет рассмотрено далее.

## Основная функция

Основная функция этого модуля - преобразовать фазу с одной частотой в фазу с другой частотой, при этом имея возможность выразить соотношение этих частот в виде соотношения двух чисел. Проще всего объяснить это с помощью графика:

<img align='middle' src='divider-main-function.svg'/>

Здесь видно, что установка соотношения в значение 4:1 делает выходной сигнал в 4 раза медленнее и превращает его в фазу целого такта, размером 4 доли, а установка соотношения в значение 4:5 позволяет получить полиритм.

## Свинг

Фактор свинга позволяет изменить продолжительность каждой второй фазы относительно каждой первой:

<img align='middle' src='divider-swing.svg'/>

<img align='right' src='divider-blueprint.svg' class='md-only'/>

## Главная панель

<!---
start: legend
-->

<!---
start: legend-group
slug: inputs
-->

### Входы

* <!---
  x: 8
  y: 191
  slug: swing
  type: labeled-socket
  -->
  <a name="inputs-swing" href='#inputs-swing'>Swing</a> - CV фактора свинга. Диапазон -5V - +5V.
  > Поданное напряжение масштабируется на оставшийся диапазон в зависимости от того, в каком положении находится [регулятор свинга](#controls-swing).

* <!---
  x: 8
  y: 233
  slug: from
  type: labeled-socket
  -->
  <a name="inputs-from" href='#inputs-from'>From</a> - CV для числителя соотношения.
  > Влияет нелинейно на финальное значение числителя в диапазоне от 1 до заданного с помощью регулятора. Диапазон CV от 0V до 10V. Например, если регулятор числителя выставлен в положение 8, то 10V на входе CV будет равно числителю 8, 5V - числителю 4, 2.5V - числителю 2 и 0V - числителю 1. То же самое относится к следующему входу [To](#inputs-to).

* <!---
  x: 42
  y: 233
  slug: to
  type: labeled-socket
  -->
  <a name="inputs-to" href='#inputs-to'>To</a> - CV для знаменателя соотношения.

* <!---
  x: 8
  y: 275
  slug: phase
  type: labeled-socket
  -->
  <a name="inputs-phase" href='#inputs-phase'>Phase</a> - исходная фаза. Диапазон 0V - 10V.

* <!---
  x: 42
  y: 275
  slug: reset
  type: labeled-socket
  -->
  <a name="inputs-reset" href='#inputs-reset'>Reset</a> - сброс фазы.

<!---
end: legend-group
-->

<!---
start: legend-group
slug: outputs
-->

### Выходы

* <!---
  x: 8
  y: 320
  slug: clock
  type: labeled-socket
  -->
  <a name="outputs-clock" href='#outputs-clock'>CLK</a> - импульсообразный сигнал. Импульсы генерируются с каждым новым началом результирующей фазы и с учётом её свинга.
  > Выход может быть переключён из режима коротких импульсов в режим гейта (Gate Mode) при помощи меню правой кнопки. В режиме гейта продолжительность импульсов становится равной половине фазы.

* <!---
  x: 42
  y: 320
  slug: phase
  type: labeled-socket
  -->
  <a name="outputs-phase" href='#outputs-phase'>Phase</a> - результирующая фаза. Диапазон 0V - 10V.

<!---
end: legend-group
-->

<!---
start: legend-group
slug: controls
-->

### Управление

* <!---
  x: 12
  y: 39
  slug: from
  type: cross-knob
  -->
  <a name="controls-from" href='#controls-from'>From</a> - числитель соотношения.

* <!---
  x: 12
  y: 123
  slug: to
  type: cross-knob
  -->
  <a name="controls-to" href='#controls-to'>To</a> - знаменатель соотношения.

* <!---
  x: 42
  y: 191
  slug: swing
  type: knob-25
  -->
  <a name="controls-swing" href='#controls-swing'>Swing</a> - фактор свинга для результирующей фазы.
  > Регулятор имеет вспомогательные отметки для положений 25%, 33%, 50%, 66% и 75%.

<!---
end: legend-group
-->

<!---
start: legend-group
slug: indicators
-->

### Индикация

* <!---
  x: 9
  y: 94
  slug: ratio
  type: ratio-display
  -->
  <a name="indicators-ratio" href='#indicators-ratio'>Ratio</a> - текущее соотношение частоты входящей фазы к частоте результирующей.

<!---
end: legend-group
-->

<!---
end: legend
-->

<!---
end: affixing
-->
