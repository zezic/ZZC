<template lang='pug'>
.module-page
  rack(:crumbs='rackCrumbs')
  crumbsbar#article(theme='yellow', :crumbs='crumbs')
  container
    .docs-layout
      docs-section
</template>

<script>
import Container from '~/components/Container'
import Crumbsbar from '~/components/Crumbsbar'
import DocsSection from '~/components/DocsSection'
import Rack from '~/components/Rack'

export default {
  components: {
    Container,
    Crumbsbar,
    DocsSection,
    Rack
  },
  data: () => ({
    crumbs: [
      { url: '/',
        title: 'Clock Manipulation' },
      { url: '/module',
        title: 'Clock' },
    ],
    rackCrumbs: [
      { url: '/',
        title: 'Clock Manipulation' }
    ]
  })
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";

.docs-layout {
  display: flex;
  flex-wrap: wrap;
  margin-bottom: 10px;

  @include phone {
    margin-top: 40px;

    & > * {
      margin-bottom: 40px;
    }
  }
  @include tablet {
    margin-top: 60px;
    margin-left: -30px;
    margin-right: -30px;

    & > * {
      padding-left: 30px;
      padding-right: 30px;
      margin-bottom: 50px;
    }
  }
}
</style>
