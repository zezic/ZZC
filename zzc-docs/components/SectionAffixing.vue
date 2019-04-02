<template lang='pug'>
.section-affixing
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
        template(v-for='(item, idx) in article')
          ttl(v-if='item.type === "heading"', :level='item.depth') {{ item.text }}
          subttl.subtitle(v-else-if='item.type === "blockquote" && idx < 3') {{ item.items[0].text }}
          template(v-else-if='item.type === "legend"')
            affix-legend.legend(
              :widgetGroups='item.items',
              :blueprintRect='blueprintRect',
              :spaghettiEnabledFor='spaghettiEnabledFor',
              @spaghettiRequest='onSpaghettiRequest',
              @spaghettiUnrequest='onSpaghettiUnrequest'
            )
          .md-item(v-else, v-html='renderToken(item)')
</template>

<script>
import marked from 'marked'

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
  name: 'section-affixing',
  props: {
    affixed: {
      type: Object,
      required: true
    },
    article: {
      type: Array,
      required: true
    }
  },
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
    widgetGroups () {
      if (!this.article) { return [] }
      return this.article.find(item => item.type === 'legend').items
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
    },
    renderToken (token) {
      const tokensArray = [token]
      tokensArray.links = Object.create(null)
      return marked.parser(tokensArray)
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
    margin-bottom: 40px;
  }
}

.legend {
  &:not(:first-child) {
    margin-top: 50px;
  }
}
</style>

