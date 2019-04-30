<template lang='pug'>
.module-page
  rack(
    :crumbs='rackCrumbs',
    :modules='[module]'
  )
  crumbsbar#article(theme='yellow', :crumbs='crumbs')
  container
    .docs-layout
      docs-section(
        v-for='(section, idx) in structure',
        :key='idx'
        :section='section',
        :moduleSlug='$route.params.moduleSlug'
      )
</template>

<script>
import Container from '~/components/Container'
import Crumbsbar from '~/components/Crumbsbar'
import DocsSection from '~/components/DocsSection'
import Rack from '~/components/Rack'
import MarkdownParser from '~/lib/markdown-parser'
import categories from '~/lib/categories'
import modules from '~/lib/modules'

const mdParser = new MarkdownParser()

export default {
  components: {
    Container,
    Crumbsbar,
    DocsSection,
    Rack
  },
  async asyncData ({ $axios, app, params }) {
    const locale = app.i18n.locale
    const path = `/markdown/${params.moduleSlug}/${params.moduleSlug}${locale ? '.' + locale : ''}.md`
    console.log(path)
    const markdown = await $axios.$get(path)
    return { markdown }
  },
  data: () => ({
  }),
  head () {
    return {
      title: `ZZC | ${this.module.name}`
    }
  },
  computed: {
    structure () {
      return mdParser.parse(this.markdown)
    },
    category () {
      return categories.find(category => {
        return category.slug === this.$route.params.categorySlug
      })
    },
    module () {
      return modules.find(module => module.slug === this.$route.params.moduleSlug)
    },
    rackCrumbs () {
      return [
        { url: this.localePath({ name: 'categorySlug', params: { categorySlug: this.$route.params.categorySlug } }),
          title: this.category.name[this.$i18n.locale] }
      ]
    },
    crumbs () {
      return [
        ...this.rackCrumbs,
        { url: this.$route,
          title: this.module.name }
      ]
    }
  },
  mounted () {
  }
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";

.pre {
  font-family: inherit;
}

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
