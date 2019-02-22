<template lang='pug'>
nuxt-link.logo(:to='{name: "index"}')
  zzc-logo.zzc-logo
</template>

<script>
import ZzcLogo from '~/assets/images/zzc-logo.svg?inline'

export default {
  components: {
    ZzcLogo
  }
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/utils.scss";

.logo {
  height: 30px;
  @include center;

  .zzc-logo {
    vertical-align: top;
  }
}
</style>
