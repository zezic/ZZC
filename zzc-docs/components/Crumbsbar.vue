<template lang='pug'>
.crumbsbar.theme-gray
  .yellow-line
  container(:padding='false')
    .crumbsbar-layout
      logo.logo-small
      arrow.arrow
      nuxt-link.crumb(to='/') Clock Manipulation
</template>

<script>
import Arrow from '~/assets/images/icons/arrow-right.svg?inline'
import Container from '~/components/Container'
import Logo from '~/components/Logo'

export default {
  components: {
    Arrow,
    Container,
    Logo
  }
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";
@import "~/assets/sass/colors.scss";
@import "~/assets/sass/utils.scss";
$color-gray-crumbs-selection: #616464;

.crumbsbar {

  .yellow-line {
    display: none;
  }

  &.theme-gray {
    background-color: #747878;
    color: #fff;

    .yellow-line {
      display: block;
      height: 5px;
      background-color: $color-zzc;
    }
  }

  .crumbsbar-layout {
    display: flex;
    align-items: center;
    flex-wrap: wrap;

    .logo-small {
      width: 45px;
      height: 60px;

      @include phone {
        margin-left: 20px;
      }
      @include tablet {
        margin-left: 60px;
      }
    }

    .arrow {
      margin: 0 25px;
    }

    .crumb {
      font-size: 14px;
      height: 60px;
      @include center;
      background-color: $color-gray-crumbs-selection;
      padding: 0 20px;

      &:hover {
        text-decoration: underline;
      }
    }
  }
}
</style>
