<template lang='pug'>
.title
  component(:is='`h${level}`'): slot
</template>

<script>
export default {
  props: {
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

  h1 {
    font-size: 54px;
    margin-top: 0;
    margin-bottom: 0;

    @include phone {
      font-size: 20px;
    }
  }
  h2 {
    font-size: 20px;
    margin-top: 0;
    margin-bottom: 0;
  }
}
</style>
