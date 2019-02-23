<template lang='pug'>
.rack
  crumbsbar
  container
    .rack-layout
      module
</template>

<script>
import Container from '~/components/Container'
import Crumbsbar from '~/components/Crumbsbar'
import Module from '~/components/Module'

export default {
  components: {
    Container,
    Crumbsbar,
    Module
  }
}
</script>

<style lang='scss' scoped>
$color-rack-background: #616464;

.rack {
  background-color: $color-rack-background;

  .rack-layout {
    display: flex;
    flex-wrap: wrap;
    justify-content: center;
  }
}
</style>
