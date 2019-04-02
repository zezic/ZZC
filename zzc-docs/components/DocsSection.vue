<template lang='pug'>
.docs-section
  section-affixing(
    v-if='section.type === "affixing"',
    :affixed='section.affixed',
    :article='section.items',
    :moduleSlug='moduleSlug'
  )
</template>

<script>
import SectionAffixing from '~/components/SectionAffixing'

export default {
  name: 'docs-section',
  props: {
    section: {
      type: Object,
      required: true
    },
    moduleSlug: {
      type: String,
      default: null
    }
  },
  components: {
    SectionAffixing
  }
}
</script>

<style lang='scss' scoped>
</style>
