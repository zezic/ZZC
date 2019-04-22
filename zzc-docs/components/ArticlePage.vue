<template lang='pug'>
.article-page
  rack(
    :crumbs='crumbs',
    :modules='modules'
  )
  container
    .article-layout
      article.article
        article-content.content(:struct='structure')
      aside.aside
        module-list(
          :title='$t("relatedModules")',
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
import MarkdownParser from '~/lib/markdown-parser'

const mdParser = new MarkdownParser()

export default {
  components: {
    ArticleContent,
    Container,
    ModuleList,
    Rack,
    Subttl,
    Ttl
  },
  props: {
    crumbs: {
      type: Array,
      default: () => ([])
    },
    modules: {
      type: Array,
      default: () => ([])
    },
    relatedModules: {
      type: Array,
      default: () => ([])
    },
    markdown: {
      type: String,
      required: true
    }
  },
  computed: {
    structure () {
      return mdParser.parse(this.markdown)
    }
  }
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";

.article-layout {
  display: flex;
  flex-wrap: wrap;
  margin-bottom: 60px;

  @include phone {
    margin-top: 40px;
  }
  @include tablet {
    margin-top: 50px;
    margin-left: -30px;
    margin-right: -30px;

    & > * {
      padding-left: 30px;
      padding-right: 30px;
    }
  }

  .article {
    flex-basis: 700px;
    flex-grow: 100000;
    flex-shrink: 1;
  }

  .aside {
    flex-basis: 420px;
    flex-grow: 1;
  }
}
</style>
