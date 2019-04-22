<template lang='pug'>
.md-img(:class='attrs.class')
  img(:src='attrs.src')
</template>

<script>
import parseXml from '@rgrove/parse-xml'

export default {
  props: {
    token: {
      type: Object,
      required: true
    },
    moduleSlug: {
      type: String,
      required: true
    }
  },
  name: 'md-img',
  computed: {
    attrs () {
      const doc = parseXml(this.token.text)
      const img = doc.children[0]
      return {
        src: `/markdown/${this.moduleSlug}/${img.attributes.src}`,
        class: `align-${img.attributes.align}`
      }
    }
  }
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";

.md-img {
  @include phone {
    margin-left: -20px;
    margin-right: -20px;
  }

  img {
    max-width: 100%;
  }
  &.align-middle {
    text-align: center;
  }
  &.align-left {
    img {
      float: left;
      margin-top: 15px;
      margin-right: 30px;
      margin-bottom: 15px;
    }
  }
  &.align-right {
    img {
      float: right;
      margin-top: 15px;
      margin-left: 30px;
      margin-bottom: 15px;
    }
  }
}
</style>
