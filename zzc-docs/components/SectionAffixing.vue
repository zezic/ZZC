<template lang='pug'>
.section-affixing
  affixed
    blueprint(
      slot='affix',
      ref='blueprint',
      :blueprintUrl='`/markdown/${moduleSlug}/${affixed.affixed.blueprint}`',
      :previewUrl='`/markdown/${moduleSlug}/${affixed.affixed.preview}`',
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
          md-item(v-else, :token='item', :moduleSlug='moduleSlug')
</template>

<script>
import marked from 'marked'

import Affixed from '~/components/Affixed'
import AffixLegend from '~/components/AffixLegend'
import ArticleContent from '~/components/ArticleContent'
import Blueprint from '~/components/Blueprint'
import Subttl from '~/components/SubTitle'
import Ttl from '~/components/Title'
import MdItem from '~/components/MdItem'

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
    },
    moduleSlug: {
      type: String,
      required: true
    }
  },
  components: {
    Affixed,
    AffixLegend,
    ArticleContent,
    Blueprint,
    Subttl,
    Ttl,
    MdItem
  },
  data: () => ({
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
      const legend = this.article.find(item => item.type === 'legend')
      return legend ? legend.items : []
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
</style>

