<template lang='pug'>
.rack
  crumbsbar(v-if='crumbs.length > 0', :crumbs='crumbs')
  container
    .rack-layout
      module.module(v-for='module in modules', :key='module.slug', :module='module')
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
    },
    modules: {
      typ : Array,
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
@import "~/assets/sass/breakpoints.scss";

$color-rack-background: #616464;

.rack {
  background-color: $color-rack-background;

  .rack-layout {
    display: flex;
    flex-wrap: wrap;
    justify-content: center;
    margin-left: -7.5px;
    margin-right: -7.5px;
    min-height: 380px;

    @include phone {
      margin-left: -3.75px;
      margin-right: -3.75px;
      min-height: 190px;
    }

    .module {
      margin-left: 7.5px;
      margin-right: 7.5px;

      &:hover {
        opacity: .75;
      }

      @include phone {
        margin-left: 3.75px;
        margin-right: 3.75px;
      }
    }
  }
}
</style>
