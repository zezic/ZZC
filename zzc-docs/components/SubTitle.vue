<template lang='pug'>
p.subtitle
  slot
</template>

<script>
export default {
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";

.subtitle {
  text-transform: uppercase;
  font-size: 18px;
  margin-top: 0;
  margin-bottom: 0;

  @include phone {
    font-size: 16px;
  }
}
</style>
