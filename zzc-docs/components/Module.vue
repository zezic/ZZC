<template lang='pug'>
nuxt-link.module(to='/module#article')
  img.image(src='/modules/clock/clock.svg')
</template>

<script>
export default {
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";

.module {
  max-width: 100%;

  .image {
    display: block;

    @include phone {
      max-height: 190px;
      max-width: 100%;
    }
  }
}
</style>
