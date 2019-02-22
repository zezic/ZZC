<template lang='pug'>
.container(:class='{padded: padding}')
  slot
</template>

<script>
export default {
  props: {
    padding: {
      type: Boolean,
      default: true
    }
  }
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";

.container {
  max-width: 1420px;
  margin: 0 auto;

  &.padded {
    padding-left: 20px;
    padding-right: 20px;

    @include tablet {
      padding-left: 60px;
      padding-right: 60px;
    }
  }
}
</style>
