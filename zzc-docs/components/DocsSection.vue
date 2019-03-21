<template lang='pug'>
.docs-section
  affixed
    blueprint(
      slot='affix',
      ref='blueprint',
      :widgetGroups='widgetGroups',
      :spaghettiEnabledFor='spaghettiEnabledFor',
      @spaghettiRequest='onSpaghettiRequest',
      @spaghettiUnrequest='onSpaghettiUnrequest'
    )
    template(slot='content')
      .article
        header
          ttl Clock
          subttl.subtitle The serious source of tick-tack for your virtual rack
        article-content.content(:struct='struct')
      affix-legend.legend(
        :widgetGroups='widgetGroups',
        :blueprintRect='blueprintRect',
        :spaghettiEnabledFor='spaghettiEnabledFor',
        @spaghettiRequest='onSpaghettiRequest',
        @spaghettiUnrequest='onSpaghettiUnrequest'
      )
</template>

<script>
import Affixed from '~/components/Affixed'
import AffixLegend from '~/components/AffixLegend'
import ArticleContent from '~/components/ArticleContent'
import Blueprint from '~/components/Blueprint'
import Subttl from '~/components/SubTitle'
import Ttl from '~/components/Title'

import Lorem from '~/assets/lorem-ipsum.json'

const widgets = [
  { 'title': 'V/BPS',
    'slug': 'vbps',
    'description': 'Volts per beat per second. Input affects the base BPM value. For example, adding 1V to it will raise the base BPM by 1 beat per second (60 BPM). Negative values can make transport go reverse.',
    'notices': [],
    'widget': {
      'position': { 'x': 10, 'y': 52 },
      'type': 'labeled-socket'
    } },
  { 'title': 'Run',
    'slug': 'run',
    'description': 'Phasellus quam felis, posuere eu consequat quis, tempor quis odio. Proin cursus vitae tortor ut sodales.',
    'notices': [
      'Note, that when plugged vestibulum volutpat felis porta, lacinia justo a, laoreet ex.'
    ],
    'widget': {
      'position': { 'x': 10, 'y': 145 },
      'type': 'simple-socket'
    } }
]

export default {
  name: 'docs-section',
  components: {
    Affixed,
    AffixLegend,
    ArticleContent,
    Blueprint,
    Subttl,
    Ttl
  },
  data: () => ({
    Lorem,
    blueprintRect: {
      top: 0,
      left: 0,
      bottom: 0,
      right: 0,
      width: 0,
      height: 0
    },
    widgetGroups: [
      { 'title': 'Inputs',
        'slug': 'input',
        'widgets': widgets },
      { 'title': 'Outputs',
        'slug': 'output',
        'widgets': widgets },
      { 'title': 'Parameters',
        'slug': 'parameter',
        'widgets': widgets },
      { 'title': 'Indicators',
        'slug': 'indicator',
        'widgets': widgets }
    ],
    spaghettiRequest: null,
    animationId: null,
  }),
  computed: {
    spaghettiEnabledFor () {
      if (this.spaghettiRequest) {
        return this.spaghettiRequest
      }
      return this.$route.hash ? this.$route.hash.replace('#', '') : "null"
    },
    struct () {
      return this.Lorem.text.split('\n');
    }
  },
  methods: {
    deliverBlueprintRect () {
      if (this.$refs.blueprint) {
        let blueprintRect = this.$refs.blueprint.$el.getBoundingClientRect()
        if ((blueprintRect.top !== this.blueprintRect.top) ||
            (blueprintRect.left !== this.blueprintRect.left) ||
            (blueprintRect.bottom !== this.blueprintRect.bottom) ||
            (blueprintRect.right !== this.blueprintRect.right) ||
            (blueprintRect.width !== this.blueprintRect.width) ||
            (blueprintRect.height !== this.blueprintRect.height)) {
          this.blueprintRect = {
            top: blueprintRect.top,
            left: blueprintRect.left,
            bottom: blueprintRect.bottom,
            right: blueprintRect.right,
            width: blueprintRect.width,
            height: blueprintRect.height
          }
        }
      }
      this.animationId = requestAnimationFrame(this.deliverBlueprintRect)
    },
    onSpaghettiRequest (newWidget) {
      this.spaghettiRequest = newWidget
    },
    onSpaghettiUnrequest () {
      this.spaghettiRequest = null
    }
  },
  mounted () {
    this.deliverBlueprintRect()
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

.blueprint {
  @include phone {
    display: none;
  }
}
.article {
  flex-basis: 500px;
  flex-grow: 1;
  flex-shrink: 1;

  .subtitle {
    margin-top: 10px;
  }
  .content {
    margin-top: 40px;
  }
}

.legend {
  &:not(:first-child) {
    margin-top: 50px;
  }
}
</style>
