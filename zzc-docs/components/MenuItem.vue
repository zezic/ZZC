<template lang='pug'>
a.menu-item(:href='to', v-if='typeof to === "string" && to.startsWith("http")')
  .icon(v-if='$slots.icon', :class='{solid}'): slot(name='icon')
  .text: slot
nuxt-link.menu-item(:to='to', v-else)
  .icon(v-if='$slots.icon', :class='{solid}'): slot(name='icon')
  .text: slot
</template>

<script>
export default {
  props: {
    to: {
      required: true
    },
    solid: {
      type: Boolean,
      default: false
    }
  }
}
</script>

<style lang='scss'>
.menu-item {
  .icon {
    & > * {
      display: block;
      // Anti-blur hack for Firefox
      transform: translateX(0);
    }
  }
}
</style>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";
@import "~/assets/sass/utils.scss";

.menu-item {
  color: inherit;
  font-size: 18px;
  display: flex;
  align-items: center;
  height: 30px;

  @include phone {
    flex-direction: row-reverse;
  }

  &:hover, &.nuxt-link-exact-active {
    .icon {
      opacity: 1;
    }
  }

  &:hover {
    .text {
      text-decoration: underline;
    }
  }

  .icon {
    margin-right: 16px;
    width: 24px;
    text-align: center;

    &:not(.solid) {
      opacity: .25;
    }

    @include center

    @include phone {
      flex-direction: row-reverse;
      margin-left: 16px;
      margin-right: 0;
      opacity: 1;
    }
  }

  .text {
    white-space: nowrap;
    padding-top: 0.1em;
  }
}
</style>
