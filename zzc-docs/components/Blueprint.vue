<template lang='pug'>
.blueprint
  img.blueprint-img(
    :src='blueprintUrl',
    ref='blueprint',
    :class='{dimmed}'
  )
  .previews
    template(v-for='group in widgetGroups')
      template(v-for='groupItem in group.items')
        nuxt-link.preview(
          v-if='groupItem.type === "list"',
          v-for='widget in groupItem.items',
          :key='makeGroupItemSlug(widget, group)',
          :to='{ ...$route, hash: `#${makeGroupItemSlug(widget, group)}` }',
          :style='{backgroundImage: `url(${previewUrl})`, ...styleForWidget(widget)}',
          :class='{active: spaghettiEnabledFor === makeGroupItemSlug(widget, group)}',
          @mouseenter.native='activateSpaghetti(widget, group)',
          @mouseleave.native='deactivateSpaghetti(widget, group)'
        )
</template>

<script>
import { makeGroupItemSlug } from '~/lib/shared'
import widgetRects from '~/lib/widget-rects'

export default {
  props: {
    widgetGroups: {
      type: Array,
      default: () => ([])
    },
    spaghettiEnabledFor: {
      type: String,
      required: true
    },
    blueprintUrl: {
      type: String,
      required: true
    },
    previewUrl: {
      type: String,
      required: true
    }
  },
  name: 'blueprint',
  data: () => ({
    scale: 2.0,
    animationId: null
  }),
  methods: {
    styleForWidget (widget) {
      if (!widget.options) { return {} }
      const rect = widgetRects[widget.options.type]
      const position = { x: parseInt(widget.options.x), y: parseInt(widget.options.y) }
      return {
        transform: `scale(${this.scale}) translate(${position.x + rect.x}px, ${position.y + rect.y}px)`,
        width: `${rect.width}px`,
        height: `${rect.height}px`,
        backgroundPosition: `-${position.x + rect.x}px -${position.y + rect.y}px`,
        borderRadius: rect.radius
      }
    },
    updateScale () {
      if (this.$refs.blueprint) {
        const blueprintRect = this.$refs.blueprint.getBoundingClientRect()
        this.scale = blueprintRect.height / 380
      }
      requestAnimationFrame(this.updateScale)
    },
    cleanupAnimation () {
      if (this.animationId) {
        cancelAnimationFrame(this.animationId)
      }
    },
    activateSpaghetti (widget, group) {
      this.$emit('spaghettiRequest', this.makeGroupItemSlug(widget, group))
    },
    deactivateSpaghetti (widget, group) {
      this.$emit('spaghettiUnrequest')
    },
    makeGroupItemSlug
  },
  computed: {
    previewsStyle () {
      return {
        transform: `scale(${this.scale})`,
        transformOrigin: `0 0`
      }
    },
    dimmed () {
      return this.widgetGroups.map(({ slug }) => {
        return this.spaghettiEnabledFor.indexOf(`${slug}-`) !== -1
      }).filter(hit => hit).length > 0
    }
  },
  mounted () {
    this.updateScale()
  },
  beforeDestroy () {
    this.cleanupAnimation()
  }
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";
@import "~/assets/sass/colors.scss";

.blueprint {
  position: relative;

  .blueprint-img {
    height: calc(380px * 2);
    margin-right: 60px;
    max-height: calc(100vh - 120px);
    flex-shrink: 0;
    display: block;
    transition: opacity .2s ease;

    &.dimmed {
      opacity: .3;
    }
  }

  .previews {
    position: absolute;
    left: 0;
    top: 0;

    .preview {
      position: absolute;
      left: 0;
      top: 0;
      // border: 2px solid $color-zzc;
      // box-sizing: content-box;
      opacity: 0;
      cursor: pointer;
      transform-origin: 0 0;

      &:focus {
        outline: 0;
      }

      &.active {
        opacity: 1;
        box-shadow: 0 0 0 2px $color-zzc;
      }
    }
  }
}
</style>
