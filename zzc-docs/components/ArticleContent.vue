<template lang='pug'>
.content
  template(v-for='(item, idx) in struct')
    ttl(v-if='item.type === "heading"', :level='item.depth') {{ item.text }}
    subttl.subtitle(v-else-if='item.type === "blockquote" && idx < 3') {{ item.items[0].text }}
    md-item(v-else, :token='item', moduleSlug='index')
</template>

<script>
import Subttl from '~/components/SubTitle'
import Ttl from '~/components/Title'
import MdItem from '~/components/MdItem'

export default {
  components: {
    Subttl,
    Ttl,
    MdItem
  },
  props: {
    struct: {
      type: Array,
      default: () => ([])
    }
  }
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";

.content {
  font-family: 'Montserrat';

  p {
    opacity: .75;
    line-height: 1.65;
    font-size: 14px;
    margin-top: 0;
    margin-bottom: 0;

    &:not(:last-child) {
      margin-bottom: 1.5em;
    }

    @include phone {
      font-size: 12px;
    }
  }
}
</style>
