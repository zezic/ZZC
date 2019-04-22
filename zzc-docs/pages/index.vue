<template lang='pug'>
.index-page
  article-page(
    :markdown='markdown',
    :crumbs='[]',
    :modules='modules',
    :relatedModules='modules'
  )
</template>

<script>
import ArticlePage from '~/components/ArticlePage'
import modules from '~/lib/modules.js'

export default {
  components: {
    ArticlePage
  },
  async asyncData ({ $axios, app, params }) {
    const locale = app.i18n.locale
    const path = `/markdown/index.${locale}.md`
    const markdown = await $axios.$get(path)
    return { markdown }
  },
  data: () => ({
    modules
  })
}
</script>
