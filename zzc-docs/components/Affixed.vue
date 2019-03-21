<template lang='pug'>
.affixed
  .affix(
    :class='affixClass',
    :style='affixStyle',
    ref='affix'
  )
    slot(name='affix')
  .content(ref='content')
    slot(name='content')
</template>

<script>
import Ttl from '~/components/Title'

export default {
  name: 'affixed',
  components: {
    Ttl
  },
  data: () => ({
    animationId: null,
    offset: 0,
    affixClass: ''
  }),
  methods: {
    updateTransform () {
      let contentRect = this.$refs.content.getBoundingClientRect()
      let affixRect = this.$refs.affix.getBoundingClientRect()
      if (contentRect.top >= 60) {
        this.offset = 0
      } else {
        if (contentRect.bottom > window.innerHeight - 60) {
          this.offset = 60 - contentRect.top
        } else {
          this.offset = Math.min(50 - contentRect.top, contentRect.height - affixRect.height)
        }
      }
      this.animationId = requestAnimationFrame(this.updateTransform);
    }
  },
  computed: {
    affixStyle () {
      return {
        transform: `translateY(${this.offset}px)`
      }
    }
  },
  mounted () {
    if (navigator.userAgent.match(/Firefox/i)) {
      this.affixClass = 'smooth'
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

.affixed {
  display: flex;
  align-items: flex-start;

  .affix {
    &.smooth {
      transition: transform .2s linear;
    }
  }
}
</style>
