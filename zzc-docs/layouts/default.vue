<template lang='pug'>
.layout
  header
    site-header
  .yellow-line
  .page-wrapper
    nuxt
  footer
    site-footer
</template>

<script>
import SiteHeader from '~/components/SiteHeader'
import SiteFooter from '~/components/SiteFooter'

export default {
  components: {
    SiteHeader,
    SiteFooter
  }
}
</script>


<style lang='scss'>
@import "~/assets/sass/fonts.sass";

* {
  box-sizing: border-box;
}
html {
  background: #fff;
  font-family: 'agave';
}
a {
  color: inherit;
  text-decoration: none;
}
</style>

<style lang='scss' scoped>
@import "~/assets/sass/colors.sass";

.layout {
  display: flex;
  flex-direction: column;
  min-height: 100vh;

  .yellow-line {
    height: 5px;
    background-color: $color-zzc;
  }

  footer {
    margin-top: auto;
  }
}
</style>
