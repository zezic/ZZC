<template lang='pug'>
.blueprint
  img.blueprint-img(
    src='/images/modules/clock-blueprint.svg',
    ref='blueprint'
  )
  .previews(:style='previewsStyle')
    template(v-for='group in widgetGroups')
      .preview(
        v-for='widget in group.widgets',
        :style='{backgroundImage: "url(/images/modules/clock.svg)", ...styleForWidget(widget)}'
      )
</template>

<script>
const widgetRects = {
  'labeled-socket': {
    width: 29.5,
    height: 43,
    x: -2.4,
    y: -15,
    radius: '3px'
  },
  'simple-socket': {
    width: 29.5,
    height: 31.5,
    x: -2.4,
    y: -3.5,
    radius: '3px'
  }
}
const border = 2

export default {
  props: {
    widgetGroups: {
      type: Array,
      default: () => ([])
    }
  },
  name: 'blueprint',
  data: () => ({
    scale: 2.0,
    animationId: null
  }),
  methods: {
    styleForWidget (widget) {
      const rect = widgetRects[widget.widget.type]
      const position = widget.widget.position
      return {
        marginLeft: `${position.x}px`,
        marginTop: `${position.y}px`,
        transform: `translate(${rect.x - border}px, ${rect.y - border}px)`,
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
    }
  },
  computed: {
    previewsStyle () {
      return {
        transform: `scale(${this.scale})`,
        transformOrigin: `0 0`
      }
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
  }

  .previews {
    position: absolute;
    left: 0;
    top: 0;

    .preview {
      position: absolute;
      left: 0;
      top: 0;
      border: 2px solid $color-zzc;
      box-sizing: content-box;
    }
  }
}
</style>
