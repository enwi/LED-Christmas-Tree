<template>
  <div class="main">
    <table class="container">
      <tr>
        <td class="border">Wifi mode</td>
        <td class="border">
          <Selector
            :value="wifi_mode"
            @input="set_wifi_mode"
            :choices="[
              { name: 'Client', value: 'client' },
              { name: 'Access point', value: 'ap' },
            ]"
          ></Selector>
        </td>
      </tr>
      <tr v-if="config.wifi.ap_enabled">
        <td class="border">Access point</td>
        <td class="border">
          <table class="container-full">
            <tr>
              <td>SSID</td>
              <td><TextInput v-model="config.wifi.ap_ssid"></TextInput></td>
            </tr>
            <tr>
              <td>Password</td>
              <td>
                <TextInput
                  :hidden="true"
                  v-model="config.wifi.ap_password"
                ></TextInput>
              </td>
            </tr>
          </table>
        </td>
      </tr>
      <tr v-if="config.wifi.client_enabled">
        <td class="border">Wifi client</td>
        <td class="border">
          <table class="container-full">
            <tr>
              <td>SSID</td>
              <td><TextInput v-model="config.wifi.client_ssid"></TextInput></td>
            </tr>
            <tr>
              <td>Password</td>
              <td>
                <TextInput
                  :hidden="true"
                  v-model="config.wifi.client_password"
                ></TextInput>
              </td>
            </tr>
            <tr>
              <td>DHCP</td>
              <td>
                <SlideSwitch
                  v-model="config.wifi.client_dhcp_enabled"
                ></SlideSwitch>
              </td>
            </tr>
            <template v-if="!config.wifi.client_dhcp_enabled">
              <tr>
                <td>IP</td>
                <td><TextInput v-model="config.wifi.client_ip"></TextInput></td>
              </tr>
              <tr>
                <td>Netmask</td>
                <td>
                  <TextInput v-model="config.wifi.client_mask"></TextInput>
                </td>
              </tr>
              <tr>
                <td>Gateway</td>
                <td>
                  <TextInput v-model="config.wifi.client_gateway"></TextInput>
                </td>
              </tr>
              <tr>
                <td>DNS</td>
                <td>
                  <TextInput v-model="config.wifi.client_dns"></TextInput>
                </td>
              </tr>
            </template>
          </table>
        </td>
      </tr>
      <tr>
        <td class="border">MQTT</td>
        <td class="border">
          <table class="container-full">
            <tr>
              <td>Enabled</td>
              <td style="width: 100%">
                <SlideSwitch v-model="config.mqtt.enabled"></SlideSwitch>
              </td>
            </tr>
            <template v-if="config.mqtt.enabled">
              <tr>
                <td>Server</td>
                <td><TextInput v-model="config.mqtt.server"></TextInput></td>
              </tr>
              <tr>
                <td>Port</td>
                <td><NumberInput v-model="config.mqtt.port"></NumberInput></td>
              </tr>
              <tr>
                <td>Id</td>
                <td><TextInput v-model="config.mqtt.id"></TextInput></td>
              </tr>
              <tr>
                <td>User</td>
                <td><TextInput v-model="config.mqtt.user"></TextInput></td>
              </tr>
              <tr>
                <td>Password</td>
                <td>
                  <TextInput
                    :hidden="true"
                    v-model="config.mqtt.password"
                  ></TextInput>
                </td>
              </tr>
            </template>
          </table>
        </td>
      </tr>

      <tr>
        <td class="border" style="border-color: #ef5350; border-width: 2px">
          OTA
        </td>
        <td class="border" style="border-color: #ef5350; border-width: 2px">
          <form method="POST" action="/ota" enctype="multipart/form-data">
            <label class="file">
              <input
                type="file"
                accept=".bin, application/octet-stream"
                name="data"
              />
              <span class="file-custom"></span>
            </label>
            <input
              type="submit"
              name="upload"
              value="Upload"
              title="Upload Files"
            />
          </form>
        </td>
      </tr>

      <tr>
        <td colspan="2">
          <button @click="api_post_save">Save config</button>
        </td>
      </tr>
    </table>
  </div>
</template>

<script>
// import SlideSwitch from "@/components/inputs/SlideSwitch";
import Selector from "@/components/inputs/Selector";
import { api_mixin } from "@/api_mixin";
import SlideSwitch from "@/components/inputs/SlideSwitch";
import TextInput from "@/components/inputs/TextInput";
import NumberInput from "@/components/inputs/NumberInput";

export default {
  name: "PageConfig",
  mixins: [api_mixin],
  components: {
    NumberInput,
    TextInput,
    SlideSwitch,
    Selector,
    // Slider
  },
  computed: {
    wifi_mode() {
      return this.config.wifi.ap_enabled ? "ap" : "client";
    },
  },
  data() {
    return {};
  },
  methods: {
    set_wifi_mode(value) {
      if (value === "ap") {
        this.config.wifi.ap_enabled = true;
        this.config.wifi.client_enabled = false;
      } else {
        this.config.wifi.ap_enabled = false;
        this.config.wifi.client_enabled = true;
      }
    },
  },

  mounted() {},
  beforeDestroy() {},
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
.container-full {
  width: 100%;
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

button {
  background-color: #388e3c;
  color: #fcfcfc;

  padding: 10px;
  font-size: 1rem;

  width: 100%;
  border: 1px solid #2e9035;
  border-radius: 5px;

  cursor: pointer;
}
</style>
