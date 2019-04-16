<template lang='pug'>
nuxt-link.module(:to='localePath({ name: "categorySlug-moduleSlug", params: { categorySlug, moduleSlug } })')
  img.image(:src='`/modules/${moduleSlug}/${moduleSlug}.svg`')
</template>

<script>
export default {
  props: {
    categorySlug: {
      type: String,
      required: true
    },
    moduleSlug: {
      type: String,
      required: true
    }
  }
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
