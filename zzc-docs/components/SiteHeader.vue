<template lang='pug'>
.site-header
  .header-bg
    zzc-outline.logo-outline
  container
    .header-layout
      .coupling
        .logo-container
          logo
        .menu-container
          main-menu
          //- .group
          //-   .separator: .dot
          //-   menu-item(:to='localePath({ name: "index" })') {{ $t('menu.all') }}
          //-   menu-item(:to='localePath({ name: "categorySlug", params: { categorySlug: "free" } })') {{ $t('menu.free') }}
          //-   menu-item(:to='localePath({ name: "categorySlug", params: { categorySlug: "paid" } })') {{ $t('menu.paid') }}
          .group
            .separator: .dot
            menu-item(:to='switchLocalePath("en")') En
            menu-item(:to='switchLocalePath("ru")') Ru
          //- .group
          //-   .separator: .dot
          //-   menu-item(to='/about') About
</template>

<script>
import Container from '~/components/Container'
import Logo from '~/components/Logo'
import MainMenu from '~/components/MainMenu'
import MenuItem from '~/components/MenuItem'
import ZzcOutline from '~/assets/images/zzc-logo-outline.svg?inline'

export default {
  components: {
    Container,
    Logo,
    MainMenu,
    MenuItem,
    ZzcOutline
  }
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";
@import "~/assets/sass/utils.scss";

.site-header {
  color: #424747;
  position: relative;

  .header-bg {
    @include stretch;
    overflow: hidden;

    .logo-outline {
      position: absolute;
      height: 800%;
      width: 150%;
      top: -350%;
      opacity: 0.08;
      left: 50%;
      margin-left: -75%;

      @include tablet {
        max-width: 1400px;
        margin-left: -800px;
      }
    }
  }

  .header-layout {
    position: relative;
    display: flex;
    align-items: center;

    @include tablet {
      min-height: 145px;
      padding-top: 20px;
      padding-bottom: 20px;
    }

    .coupling {
      display: flex;
      align-items: flex-start;
      flex-grow: 1;

      .logo-container {
        padding-top: 10px;
        padding-bottom: 10px;

        @include phone {
          min-height: 60px;
          width: 100%;
          @include center;
        }
      }

      .menu-container {
        display: flex;
        flex-wrap: wrap;
        justify-content: flex-end;
        margin-left: auto;
        padding-left: 50px;

        @include phone {
          display: none;
        }

        & > * {
          &:not(.main-menu) {
            margin-top: 10px;
            margin-bottom: 10px;
          }
        }
        .group {
          display: flex;

          & > * {
            &:not(:last-child) {
              &:not(.separator) {
                margin-right: 30px;
              }
            }
          }
        }

        .separator {
          height: 30px;
          width: 100px;
          @include center;

          .dot {
            height: 4px;
            width: 4px;
            border-radius: 50%;
            background-color: currentColor;
            opacity: 0.25;
          }
        }
      }
    }
  }
}
</style>
