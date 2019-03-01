<template lang='pug'>
.diagram
  img.blueprint(src='~/assets/images/modules/clock-blueprint.svg')
  .list
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
    ]
  })
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";

.diagram {
  display: flex;

  .blueprint {
    height: calc(380px * 2);
    margin-right: 60px;

    @include phone {
      display: none;
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
