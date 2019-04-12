<template lang='pug'>
.title
  component(:is='`h${level}`', :class='{center}'): slot
</template>

<script>
export default {
  props: {
    center: {
      type: Boolean,
      default: false
    },
    level: {
      type: Number,
      default: 1
    }
  }
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";

.title {
  font-family: 'Montserrat';

  h2, h3, h4, h5, h6 {
    margin-top: 1.0em;
    margin-bottom: 0.5em;
  }
  h1, h2, h3, h4, h5, h6 {
    &.center {
      display: flex;
      align-items: center;
    }
  }

  h1 {
    margin-top: -8px;
    margin-bottom: 0;
    font-size: 54px;

    @include phone {
      font-size: 28px;
    }
  }
  h2 {
    font-size: 24px;

    @include phone {
      font-size: 20px;
    }
  }
  h3 {
    font-size: 18px;

    @include phone {
      font-size: 16px;
    }
  }
}
</style>
