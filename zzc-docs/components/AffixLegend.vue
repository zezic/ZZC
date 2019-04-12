<template lang='pug'>
.affix-legend(ref='legend')
  section(v-for='group in widgetGroups')
    template(v-for='item in group.items')
      .title-layout(v-if='item.type === "heading"')
        ttl(:level='3', :center='true')
          component.icon(:is='`${group.slug}-icon`')
          span {{ item.text }}
      ul.widgets(v-else)
        li.widget(
          v-for='widget in item.items',
          :id='makeGroupItemSlug(widget, group)',
        )
          widget-legend(
            :widget='widget',
            :slug='makeGroupItemSlug(widget, group)',
            :blueprintRect='blueprintRect',
            :spaghettiEnabled='spaghettiEnabledFor === makeGroupItemSlug(widget, group)',
            @spaghettiRequest='onSpaghettiRequest(widget, group)',
            @spaghettiUnrequest='onSpaghettiUnrequest(widget, group)',
          )
</template>

<script>
import { makeGroupItemSlug } from '~/lib/shared'
import InputsIcon from '~/assets/images/icons/input.svg?inline'
import OutputsIcon from '~/assets/images/icons/output.svg?inline'
import ControlsIcon from '~/assets/images/icons/knob.svg?inline'
import IndicatorsIcon from '~/assets/images/icons/levels.svg?inline'
import Ttl from '~/components/Title'
import WidgetLegend from '~/components/WidgetLegend'

export default {
  name: 'affix-legend',
  components: {
    InputsIcon,
    OutputsIcon,
    ControlsIcon,
    IndicatorsIcon,
    Ttl,
    WidgetLegend
  },
  props: {
    blueprintRect: {
      type: Object,
      required: true
    },
    widgetGroups: {
      type: Array,
      default: () => ([])
    },
    spaghettiEnabledFor: {
      type: String,
      required: true
    }
  },
  data: () => ({
    spaghettiRequest: null
  }),
  methods: {
    onSpaghettiRequest (widget, group) {
      this.spaghettiRequest = this.makeGroupItemSlug(widget, group)
      this.$emit('spaghettiRequest', this.spaghettiRequest)
    },
    onSpaghettiUnrequest () {
      this.spaghettiRequest = null
      this.$emit('spaghettiUnrequest', this.spaghettiRequest)
    },
    makeGroupItemSlug
  }
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";

.affix-legend {
  min-width: 280px;
  position: relative;

  section {

    .title-layout {
      display: flex;
      align-items: center;
      margin-bottom: 20px;

      .icon {
        transform: translateX(0);
        margin-right: 15px;
      }
    }

    .widgets {
      padding-left: 0;
      margin: 0;

      .widget {
        list-style: none;
      }
    }
  }
}
</style>

