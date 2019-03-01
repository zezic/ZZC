<template lang='pug'>
.diagram
  img.blueprint(
    src='~/assets/images/modules/clock-blueprint.svg',
    :class='blueprintClass',
    :style='blueprintStyle',
    ref='blueprint'
  )
  .list(ref='list')
    section(v-for='group in widgetGroups')
      .title-layout
        component.icon(:is='`${group.slug}-icon`')
        ttl(:level='3') {{ group.title }}
      ul.widgets
        li.widget(v-for='widget in group.widgets')
          nuxt-link.title(to='#') {{ widget.title }}
          span.description {{ widget.description }}
          .notice(v-for='notice in widget.notices')
            notice-icon.notice-icon
            .notice-text {{ notice }}
</template>

<script>
import Ttl from '~/components/Title'
import InputsIcon from '~/assets/images/icons/input.svg?inline'
import OutputsIcon from '~/assets/images/icons/output.svg?inline'
import ParametersIcon from '~/assets/images/icons/knob.svg?inline'
import IndicatorsIcon from '~/assets/images/icons/levels.svg?inline'
import NoticeIcon from '~/assets/images/icons/notice.svg?inline'

const widgets = [
  { 'title': 'Reset',
    'description': 'Integer nec faucibus eros. Aliquam nec consectetur purus, at laoreet nulla.',
    'notes': [] },
  { 'title': 'Run',
    'description': 'Phasellus quam felis, posuere eu consequat quis, tempor quis odio. Proin cursus vitae tortor ut sodales.',
    'notices': [
      'Note, that when plugged vestibulum volutpat felis porta, lacinia justo a, laoreet ex.'
    ] }
]

export default {
  name: 'diagram',
  components: {
    Ttl,
    InputsIcon,
    OutputsIcon,
    ParametersIcon,
    IndicatorsIcon,
    NoticeIcon
  },
  data: () => ({
    widgetGroups: [
      { 'title': 'Inputs',
        'slug': 'inputs',
        'widgets': widgets },
      { 'title': 'Outputs',
        'slug': 'outputs',
        'widgets': widgets },
      { 'title': 'Parameters',
        'slug': 'parameters',
        'widgets': widgets },
      { 'title': 'Indicators',
        'slug': 'indicators',
        'widgets': widgets }
    ],
    animationId: null,
    offset: 0,
    blueprintClass: ''
  }),
  methods: {
    updateTransform () {
      let listRect = this.$refs.list.getBoundingClientRect()
      let bpRect = this.$refs.blueprint.getBoundingClientRect()
      if (listRect.top >= 50) {
        this.offset = 0
      } else {
        if (listRect.bottom > window.innerHeight - 60) {
          this.offset = 50 - listRect.top
        } else {
          this.offset = Math.min(50 - listRect.top, listRect.height - bpRect.height - 10)
        }
      }
      this.animationId = requestAnimationFrame(this.updateTransform);
    }
  },
  computed: {
    blueprintStyle () {
      return {
        transform: `translateY(${this.offset}px)`
      }
    }
  },
  mounted () {
    if (navigator.userAgent.match(/Firefox/i)) {
      this.blueprintClass = 'smooth'
    }
    this.updateTransform()
  },
  beforeDestroy () {
    if (this.animationId) {
      cancelAnimationFrame(this.animationId)
    }
  }
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";

.diagram {
  display: flex;

  .blueprint {
    height: calc(380px * 2);
    margin-right: 60px;
    margin-top: 10px;
    max-height: calc(100vh - 120px);
    flex-shrink: 0;
    will-change: transform;

    @include phone {
      display: none;
    }

    &.smooth {
      transition: transform .2s linear;
    }
  }

  .list {
    min-width: 280px;

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

          &:not(:last-child) {
            margin-bottom: 10px;
          }

          .title {
            text-transform: uppercase;
            font-size: 18px;

            &:hover {
              text-decoration: underline;
            }

            @include phone {
              font-size: 16px;
            }
          }

          .description {
            font-family: 'Montserrat';
            font-size: 14px;
            opacity: .75;

            &::before {
              content: ' - '
            }

            @include phone {
              font-size: 12px;
            }
          }

          .notice {
            display: flex;
            background-color: #f6f6f6;
            border-radius: 4px;
            padding: 15px;
            align-items: center;
            margin-top: 15px;

            .notice-icon {
              flex-shrink: 0;
              opacity: .6;
              transform: translateX(0);
              margin-right: 15px;
            }
            .notice-text {
              font-family: 'Montserrat';
              font-size: 14px;
              opacity: .75;
              line-height: 1.35;

              @include phone {
                font-size: 12px;
              }
            }
          }
        }
      }
    }
  }
}
</style>
