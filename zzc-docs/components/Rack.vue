<template lang='pug'>
.rack
  crumbsbar(v-if='crumbs.length > 0', :crumbs='crumbs')
  container
    .rack-layout
      module
</template>

<script>
import Container from '~/components/Container'
import Crumbsbar from '~/components/Crumbsbar'
import Module from '~/components/Module'

export default {
  props: {
    crumbs: {
      type: Array,
      default: () => ([])
    }
  },
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
