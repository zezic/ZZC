<template lang='pug'>
.widget-legend
  .hover-area(
    @mouseenter='activateSpaghetti',
    @mouseleave='deactivateSpaghetti',
    :class='{highlight: spaghettiEnabledInternal}',
    ref='hoverArea'
  )
    .description
      template(v-for='item in widget.items')
        .notice(v-if='item.type === "blockquote"')
          notice-icon.notice-icon
          .notice-text
            md-item(v-for='(token, idx) in item.items', :token='token', moduleSlug='TODO', :key='idx')
        md-item(v-else, :token='item', moduleSlug='TODO')
    spaghetti.spaghetti(
      v-if='spaghettiEnabledInternal',
      :width='spaghetti.width',
      :height='spaghetti.height',
      :class='spaghettiClass'
    )
</template>

<script>
import marked from 'marked'

import Spaghetti from '~/components/Spaghetti'
import MdItem from '~/components/MdItem'
import NoticeIcon from '~/assets/images/icons/notice.svg?inline'

import widgetRects from '~/lib/widget-rects'

export default {
  props: {
    slug: {
      type: String,
      required: true
    },
    widget: {
      type: Object,
      required: true
    },
    blueprintRect: {
      type: Object,
      required: true
    },
    spaghettiEnabled: {
      type: Boolean,
      required: true
    }
  },
  components: {
    MdItem,
    Spaghetti,
    NoticeIcon
  },
  name: 'widget-legend',
  data: () => ({
    spaghetti: {
      width: 200,
      height: 100
    },
    animationId: null,
    spaghettiEnabledInternal: false
  }),
  computed: {
    spaghettiClass () {
      return this.spaghetti.height < 0 ? 'inverted' : ''
    }
  },
  methods: {
    activateSpaghetti () {
      this.$emit('spaghettiRequest')
    },
    deactivateSpaghetti () {
      this.$emit('spaghettiUnrequest')
    },
    updateSpaghetti () {
      if (!this.spaghettiEnabled) { return }
      const widgetRect = widgetRects[this.widget.options.type]
      const bpScale = this.blueprintRect.height / 380
      const hoverAreaRect = this.$refs.hoverArea.getBoundingClientRect()
      const hoverAreaMidY = hoverAreaRect.top + hoverAreaRect.height / 2
      this.spaghetti.width = hoverAreaRect.left - this.blueprintRect.left - (bpScale * this.widget.options.x) - (bpScale * (widgetRect.width + widgetRect.x))
      this.spaghetti.height = hoverAreaMidY - this.blueprintRect.top - (bpScale * this.widget.options.y) - (bpScale * (widgetRect.height / 2 + widgetRect.y))
      this.animationId = requestAnimationFrame(this.updateSpaghetti)
    },
    cleanupAnimation () {
      if (this.animationId) {
        cancelAnimationFrame(this.animationId)
      }
    },
    checkIfSpaghettiIsEnabled () {
      // Transferring actual state here to be safe when hydrating SSR-rendered page
      // because hash part of the URL is never sent to the server
      this.spaghettiEnabledInternal = this.spaghettiEnabled
      if (this.spaghettiEnabled) {
        this.updateSpaghetti()
      } else {
        this.cleanupAnimation()
      }
    },
    renderToken (token) {
      return this.renderTokens([token])
    },
    renderTokens (tokens) {
      var tokensCopy = [...tokens]
      tokensCopy.links = Object.create(null)
      return marked.parser(tokensCopy)
    }
  },
  beforeDestroy () {
    this.cleanupAnimation()
  },
  watch: {
    spaghettiEnabled () {
      this.checkIfSpaghettiIsEnabled()
    }
  },
  mounted () {
    this.checkIfSpaghettiIsEnabled()
  }
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";
@import "~/assets/sass/colors.scss";

.widget-legend {

  .hover-area {
    position: relative;

    &.highlight {
      &::before {
        content: ' ';
        display: block;
        position: absolute;
        left: -15px;
        width: 5px;
        height: 100%;
        background-color: $color-zzc;
      }
    }

    .spaghetti {
      position: absolute;
      right: 100%;
      bottom: 50%;
      z-index: 5;

      &.inverted {
        bottom: auto;
        top: 50%;
      }

      @include phone {
        display: none;
      }
    }
  }

  .title {
    text-transform: uppercase;
    font-size: 18px;

    &:hover {
      text-decoration: underline;
    }

    @include phone {
      font-size: 16px;
    }
  }

  .description {
    font-family: 'Montserrat';
    font-size: 14px;

    @include phone {
      font-size: 12px;
    }
  }

  .notice {
    display: flex;
    background-color: #f6f6f6;
    border-radius: 4px;
    padding: 5px 15px;
    align-items: center;
    margin-top: 15px;
    margin-bottom: 15px;

    .notice-icon {
      flex-shrink: 0;
      opacity: .6;
      transform: translateX(0);
      margin-right: 15px;
    }
    .notice-text {
      font-family: 'Montserrat';
      font-size: 14px;
      opacity: .75;
      line-height: 1.35;

      @include phone {
        font-size: 12px;
      }
    }
  }
}
</style>

<style lang='scss'>
@import "~/assets/sass/colors.scss";

.widget-legend {
  .description {
    & > .md-item {
      p:first-child {
        a:first-child {
          color: $color-fg;
          font-weight: 600;
        }
      }
    }
    .notice-text .md-item p {
      margin: .5em 0;
      line-height: 1.35;
      font-size: .9em;
    }
  }
}
</style>
