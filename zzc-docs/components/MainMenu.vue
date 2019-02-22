<template lang='pug'>
.main-menu(:class='{column}')
  menu-item(:solid='column', to='/clock-manipulation') Clock Manipulation
    metronome(slot='icon')
  menu-item(:solid='column', to='/sequencers') Sequencers
    sequence(slot='icon')
  menu-item(:solid='column', to='/amps') Amps
    amp(slot='icon')
  menu-item(:solid='column', to='/mixing') Mixing
    levels(slot='icon')
  menu-item(:solid='column', to='/tools') Tools
    bulb(slot='icon')
</template>

<script>
import MenuItem from '~/components/MenuItem'

import Metronome from '~/assets/images/icons/metronome.svg?inline'
import Sequence from '~/assets/images/icons/sequence.svg?inline'
import Amp from '~/assets/images/icons/amp.svg?inline'
import Levels from '~/assets/images/icons/levels.svg?inline'
import Bulb from '~/assets/images/icons/bulb.svg?inline'

export default {
  components: {
    MenuItem,
    Metronome,
    Sequence,
    Amp,
    Levels,
    Bulb
  },
  props: {
    column: {
      type: Boolean,
      default: false
    }
  }
}
</script>

<style lang='scss' scoped>
@import '~/assets/sass/breakpoints.scss';

.main-menu {
  display: flex;
  flex-wrap: wrap;
  justify-content: flex-end;

  & > * {
    margin-top: 10px;
    margin-bottom: 10px;

    &:not(:first-child) {
      margin-left: 40px;
    }
  }

  &.column {
    flex-direction: column;

    @include tablet {
      & > * {
        margin-top: 5px;
        margin-bottom: 5px;

        &:not(:first-child) {
          margin-left: 0;
        }
      }
    }
  }
}
</style>
