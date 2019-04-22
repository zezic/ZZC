<template lang='pug'>
nuxt-link.module(:to='localePath({ name: "categorySlug-moduleSlug", params: { categorySlug: module.category.slug, moduleSlug: module.slug } })')
  img.image(:src='`/markdown/${module.slug}/${module.slug}.svg`')
</template>

<script>
export default {
  props: {
    module: {
      type: Object,
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
