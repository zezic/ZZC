<template lang='pug'>
.content
  p(v-for='item in struct') {{ item }}
</template>

<script>
export default {
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
