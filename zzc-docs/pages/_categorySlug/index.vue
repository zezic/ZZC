<template lang='pug'>
.index-page
  rack(:crumbs='crumbs')
  container
    .article-layout
      article.article
        header
          ttl Clock Manipulation
          subttl.subtitle The serious source of tick-tack for your virtual rack
        article-content.content(:struct='struct')
      aside.aside
        module-list(
          title='Related Modules',
          :modules='relatedModules'
        )
</template>

<script>
import ArticleContent from '~/components/ArticleContent'
import Container from '~/components/Container'
import Lorem from '~/assets/lorem-ipsum.json'
import ModuleList from '~/components/ModuleList'
import Rack from '~/components/Rack'
import Subttl from '~/components/SubTitle'
import Ttl from '~/components/Title'

export default {
  components: {
    ArticleContent,
    Container,
    ModuleList,
    Rack,
    Subttl,
    Ttl
  },
  data: () => ({
    Lorem,
    relatedModules: [
      { 'name': 'Clock',
        'function': 'Clock source',
        'price': 0,
        'slug': 'clock' },
      { 'name': 'Divider',
        'function': 'Phase divider',
        'price': 0,
        'slug': 'divider' }
    ],
    crumbs: [
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

.article-layout {
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
    flex-basis: 700px;
    flex-grow: 100000;
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
    flex-grow: 1;
  }
}
</style>
