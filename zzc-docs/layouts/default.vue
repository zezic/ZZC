<template lang='pug'>
.layout
  header
    site-header
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
@import "~/assets/sass/fonts.scss";

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

<style lang='scss'>
@import "~/assets/sass/breakpoints.scss";
@import "~/assets/sass/colors.scss";

.md-item {

  .md-only {
    display: none;
  }

  p {
    font-family: 'Montserrat';
    color: transparentize($color-fg, .2);
    line-height: 1.65;
    font-size: 14px;
    margin: 1.5em 0;

    @include phone {
      font-size: 12px;
    }
  }

  a {
    color: transparentize($color-fg, .0);
    font-family: 'agave';
    font-size: 1.3em;
    line-height: 1.0;

    &:hover {
      text-decoration: underline;
    }
  }
}
</style>

<style lang='scss' scoped>
@import "~/assets/sass/colors.scss";

.layout {
  display: flex;
  flex-direction: column;
  min-height: 100vh;

  .page-wrapper {
    color: #202321;
  }

  footer {
    margin-top: auto;
  }
}
</style>
