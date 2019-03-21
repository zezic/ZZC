<template lang='pug'>
.affix-legend(ref='legend')
  section(v-for='group in widgetGroups')
    .title-layout
      component.icon(:is='`${group.slug}-icon`')
      ttl(:level='3') {{ group.title }}
    ul.widgets
      li.widget(
        v-for='widget in group.widgets',
        :id='`${widget.slug}-${group.slug}`',
      )
        widget-legend(
          :widget='widget',
          :slug='`${widget.slug}-${group.slug}`',
          :blueprintRect='blueprintRect',
          :spaghettiEnabled='spaghettiEnabledFor === `${widget.slug}-${group.slug}`',
          @spaghettiRequest='onSpaghettiRequest(widget, group)',
          @spaghettiUnrequest='onSpaghettiUnrequest(widget, group)',
        )
</template>

<script>
import InputIcon from '~/assets/images/icons/input.svg?inline'
import OutputIcon from '~/assets/images/icons/output.svg?inline'
import ParameterIcon from '~/assets/images/icons/knob.svg?inline'
import IndicatorIcon from '~/assets/images/icons/levels.svg?inline'
import Ttl from '~/components/Title'
import WidgetLegend from '~/components/WidgetLegend'

export default {
  name: 'affix-legend',
  components: {
    InputIcon,
    OutputIcon,
    ParameterIcon,
    IndicatorIcon,
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
    }
  },
  data: () => ({
    spaghettiRequest: null
  }),
  methods: {
    onSpaghettiRequest (widget, group) {
      this.spaghettiRequest = `${widget.slug}-${group.slug}`
    },
    onSpaghettiUnrequest () {
      this.spaghettiRequest = null
    }
  },
  computed: {
    spaghettiEnabledFor () {
      if (this.spaghettiRequest) {
        return this.spaghettiRequest
      }
      return this.$route.hash ? this.$route.hash.replace('#', '') : null
    }
  },
  watch: {
    spaghettiEnabledFor (newWidget) {
      this.$emit('spaghettiRequest', newWidget)
    }
  }
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";

.affix-legend {
  min-width: 280px;
  position: relative;

  section {

    &:not(:last-child) {
      margin-bottom: 40px;
      @include phone {
        margin-bottom: 20px;
      }
    }

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

        &::before {
          content: "";
          display: block;
          height: 60px;
          margin: -60px 0 0;
          visibility: hidden;
          pointer-events: none;
        }

        &:not(:last-child) {
          margin-bottom: 10px;
        }
      }
    }
  }
}
</style>

