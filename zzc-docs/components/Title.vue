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

  h1, h2, h3, h4, h5, h6 {
    margin-top: 0;
    margin-bottom: 0;
  }

  h1 {
    font-size: 54px;

    @include phone {
      font-size: 28px;
    }
  }
  h2 {
    font-size: 20px;

    @include phone {
      font-size: 18px;
    }
  }
  h3 {
    font-size: 20px;

    @include phone {
      font-size: 16px;
    }
  }
}
</style>
