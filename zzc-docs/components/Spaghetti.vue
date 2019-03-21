<template lang='pug'>
svg(:width='width', :height='Math.max(Math.abs(height), 5)')
  path(:d='d')
</template>

<script>
const strokeWidth = 5
const padding = 10

export default {
  props: {
    width: {
      type: Number,
      default: 400
    },
    height: {
      type: Number,
      default: 200
    }
  },
  name: 'spaghetti',
  computed: {
    d () {
      const startY = this.height > 0 ? strokeWidth / 2 : Math.max(Math.abs(this.height), strokeWidth) - strokeWidth / 2
      const endY = this.height > 0 ? Math.max(Math.abs(this.height), strokeWidth) - strokeWidth / 2 : strokeWidth / 2
      const mid = (this.width - padding) / 2
      return `M 0,${startY} C ${mid},${startY} ${mid},${endY} ${this.width - padding},${endY}`
    }
  }
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/colors.scss";

svg {
  path {
    fill: none;
    stroke: $color-zzc;
    stroke-width: 5px;
  }
}
.spaghetti {
  pointer-events: none;

  svg {
    path {
      fill: none;
      stroke: $color-zzc;
      stroke-width: 5px;
    }
  }
}
</style>
