<template lang='pug'>
.md-item
  template(v-if='token.type')
    ul.md-ul(v-if='token.type === "list"')
      li.md-li(v-for='listItem in token.items')
        .md-html(v-for='item in listItem.items', v-html='renderToken(item)')
    .md-quote(v-else-if='token.type === "blockquote"')
      .md-html(v-for='token in token.items', v-html='renderToken(token)')
    md-img(v-else-if='isLocalImage(token)', :token='token', :moduleSlug='moduleSlug')
    .md-html(v-else, v-html='renderToken(token)')
  .error(v-else) {{ token }}
</template>

<script>
import marked from 'marked'
import parseXml from '@rgrove/parse-xml'
import MdImg from '~/components/MdImg'

export default {
  name: 'md-item',
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
  components: {
    MdImg
  },
  methods: {
    renderToken (token) {
      const tokensArray = [token]
      tokensArray.links = Object.create(null)
      return marked.parser(tokensArray)
    },
    isLocalImage (token) {
      if (token.type !== "html") { return false }
      const doc = parseXml(token.text)
      const child = doc.children[0]
      if (child.name !== "img") { return false }
      if (child.attributes.class && child.attributes.class.startsWith('md-only')) { return false }
      return !child.attributes.src.startsWith('http')
    },
    parseXml (text) {
      return parseXml(text)
    }
  }
}
</script>

<style lang='scss' scoped>
.error {
  color: red;
}
</style>

<style lang='scss'>
@import "~/assets/sass/breakpoints.scss";
@import "~/assets/sass/colors.scss";

.md-item {

  h1, h2, h3, h4, h5, h6 {
    font-family: 'Montserrat';
  }

  .md-only {
    display: none;
  }

  p {
    font-family: 'Montserrat';
    color: transparentize($color-fg, .1);
    line-height: 1.65;
    font-size: 14px;
    margin: 1.5em 0;

    @include phone {
      font-size: 12px;
    }
  }

  a {
    color: $color-link;
    // font-family: 'agave';
    // font-size: 1.3em;
    // line-height: 1.0;

    &:hover {
      text-decoration: underline;
    }
  }

  .md-ul {
    .md-li {
      .md-html {
        p {
          margin: 10px 0;
        }
      }
    }
  }

  .md-quote {
    border-left: 4px solid $color-zzc;
    padding-left: 20px;
    padding-right: 20px;

    & > .md-html > * {
      font-size: 0.8em !important;
    }
  }
}
</style>
