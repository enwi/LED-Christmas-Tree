<template>
  <div class="main">
    <table class="container">
      <tr>
        <td colspan="2" style="height: 200px">
          <Tree :value="status.lights.effect !== 0" style="margin: auto"></Tree>
        </td>
      </tr>
      <tr>
        <td colspan="2">
          <b>Wifi</b>:
          {{ status.network.wifi_ap.status === "enabled" ? "Ap" : "Client" }}
          {{
            status.network.wifi_ap.status === "enabled"
              ? "enabled"
              : status.network.wifi_client.status
          }}
          ({{
            status.network.wifi_ap.status === "enabled"
              ? status.network.wifi_ap.ip
              : status.network.wifi_client.ip
          }})
          <br />
          <b>MQTT</b>:
          {{ status.mqtt.status | capitalize }}
        </td>
      </tr>
      <tr>
        <td class="border">Effect</td>
        <td class="border">
          <Selector :value="effect" @input="set_effect" :choices="effects">
          </Selector>
        </td>
      </tr>
      <tr>
        <td class="border">Color</td>
        <td class="border">
          <Selector :value="color" @input="set_color" :choices="colors">
          </Selector>
        </td>
      </tr>
      <tr>
        <td class="border">Brightness</td>
        <td class="border">
          <Slider
            @input="api_post_set_leds"
            v-model="status.lights.brightness"
            :min="1"
            :max="8"
          ></Slider>
        </td>
      </tr>
      <tr>
        <td class="border">Speed</td>
        <td class="border">
          <Slider
            @input="api_post_set_leds"
            v-model="status.lights.speed"
            :min="0"
            :max="4"
          ></Slider>
        </td>
      </tr>
    </table>
  </div>
</template>

<script>
import Slider from "@/components/inputs/Slider";
import Selector from "@/components/inputs/Selector";
import { api_mixin } from "@/api_mixin";
import Tree from "@/components/Tree";
import { camel_to_title } from "@/utils";

export default {
  name: "PageHome",
  mixins: [api_mixin],
  components: {
    Tree,
    Selector,
    Slider,
  },
  data() {
    return {
      polling: null,
    };
  },
  computed: {
    effects() {
      return this.status.lights.effects.map((value) => {
        return {
          name: camel_to_title(value),
          value: value,
        };
      });
    },
    effect() {
      return this.effects[this.status.lights.effect].value;
    },
    colors() {
      return this.status.lights.colors.map((value) => {
        return {
          name: camel_to_title(value),
          value: value,
        };
      });
    },
    color() {
      return this.colors[this.status.lights.color].value;
    },
  },
  methods: {
    set_effect(event) {
      this.status.lights.effect = this.status.lights.effects.indexOf(event);
      this.api_post_set_leds();
    },
    set_color(event) {
      this.status.lights.color = this.status.lights.colors.indexOf(event);
      this.api_post_set_leds();
    },
  },

  mounted() {
    this.reload_status();
    this.polling = setInterval(() => {
      this.reload_status();
    }, 10000);
  },
  beforeDestroy() {
    clearInterval(this.polling);
  },
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
.main {
  width: 100%;
  display: flex;
  justify-content: center;
}
.container {
  min-width: 340px;
  width: 80%;
  border-collapse: separate;
  border-spacing: 0 10px;
}
.container tr .border {
  border: 1px solid #bfbdbd;
  border-style: solid none;
  padding: 10px;
}
.container tr .border:first-child {
  border-left-style: solid;
  border-top-left-radius: 10px;
  border-bottom-left-radius: 10px;
}
.container tr .border:last-child {
  border-right-style: solid;
  border-bottom-right-radius: 10px;
  border-top-right-radius: 10px;
}

.align-center {
  align-items: center;
}
</style>
