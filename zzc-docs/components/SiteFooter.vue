<template lang='pug'>
.site-footer
  .footer-bg
    zzc-logo.logo
  container
    .footer-layout
      .group.group-1
        logo
        menu-item(to='https://github.com/zezic') {{ $t('menu.author') }} • {{ (new Date()).getFullYear() }}
        .row
          menu-item(:to='switchLocalePath("en")') En
          menu-item(:to='switchLocalePath("ru")') Ru
        //- menu-item(to='/contact') Contact
        //- menu-item(to='/about') About
      //- .group.group-2
      //-   menu-item(:to='localePath({ name: "index" })') {{ $t('menu.all') }}
      //-   menu-item(:to='localePath({ name: "categorySlug", params: { categorySlug: "free" } })') {{ $t('menu.free') }}
      //-   menu-item(:to='localePath({ name: "categorySlug", params: { categorySlug: "paid" } })') {{ $t('menu.paid') }}
      .group.group-3
        main-menu(:column='true')
</template>

<script>
import Container from '~/components/Container'
import Logo from '~/components/Logo'
import MainMenu from '~/components/MainMenu'
import MenuItem from '~/components/MenuItem'
import ZzcLogo from '~/assets/images/zzc-logo.svg?inline'

export default {
  components: {
    Container,
    Logo,
    MainMenu,
    MenuItem,
    ZzcLogo
  }
}
</script>

<style lang='scss' scoped>
@import "~/assets/sass/breakpoints.scss";
@import "~/assets/sass/colors.scss";
@import "~/assets/sass/utils.scss";

.site-footer {
  background-color: $color-zzc;
  position: relative;

  .footer-bg {
    color: darken($color-zzc, 7%);
    overflow: hidden;
    @include stretch;

    .logo {
      position: absolute;
      width: 130%;
      left: -40%;
      margin-top: -10%;

      @include phone {
        height: 20%;
      }

      @include tablet {
        height: 100%;
        width: 100%;
        left: 50%;
        margin-left: -70%;
        margin-top: 0;
        top: -30%;
      }
    }
  }

  .footer-layout {
    color: $color-fg;
    display: flex;
    flex-wrap: wrap;
    position: relative;

    @include tablet {
      padding-top: 60px;
      padding-bottom: 60px;
    }

    @include phone {
      flex-direction: column-reverse;
      align-items: flex-end;
      padding-top: 20px;
      padding-bottom: 120px;
      padding-left: 10px;
      padding-right: 10px;
    }

    .group {
      display: flex;
      flex-direction: column;
      align-items: flex-start;

      @include tablet {
        flex-grow: 1;
        flex-basis: 0;
      }

      @include phone {
        align-items: flex-end;
      }

      & > * {
        &:not(.main-menu) {
          @include phone {
            margin-top: 10px;
            margin-bottom: 10px;
          }

          @include tablet {
            margin-top: 5px;
            margin-bottom: 5px;
          }
        }
      }

      .row {
        display: flex;

        & > *:not(:last-child) {
          margin-right: 20px;
        }
      }
    }
  }
}
</style>
