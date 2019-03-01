<template lang='pug'>
.module-page
  rack(:crumbs='rackCrumbs')
  crumbsbar(theme='yellow', :crumbs='crumbs')
  container
    .docs-layout
      article.article
        header
          ttl Clock
          subttl.subtitle The serious source of tick-tack for your virtual rack
        article-content.content(:struct='struct')
      aside.aside
        diagram
</template>

<script>
import ArticleContent from '~/components/ArticleContent'
import Container from '~/components/Container'
import Crumbsbar from '~/components/Crumbsbar'
import Diagram from '~/components/Diagram'
import Lorem from '~/assets/lorem-ipsum.json'
import ModuleList from '~/components/ModuleList'
import Rack from '~/components/Rack'
import Subttl from '~/components/SubTitle'
import Ttl from '~/components/Title'

export default {
  components: {
    ArticleContent,
    Container,
    Crumbsbar,
    Diagram,
    Rack,
    Subttl,
    Ttl
  },
  data: () => ({
    Lorem,
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
  }),
  computed: {
    struct () {
      return this.Lorem.text.split('\n');
    }
  }
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
    margin-top: 50px;
    margin-left: -30px;
    margin-right: -30px;

    & > * {
      padding-left: 30px;
      padding-right: 30px;
      margin-bottom: 50px;
    }
  }

  .article {
    flex-basis: 500px;
    flex-grow: 1;
    flex-shrink: 1;

    .subtitle {
      margin-top: 10px;
    }
    .content {
      margin-top: 30px;
    }
  }

  .aside {
    flex-basis: 420px;
    flex-grow: 100000;
  }
}
</style>
