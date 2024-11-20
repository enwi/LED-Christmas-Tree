<template>
  <label class="switch" style="margin: 0 5px">
    <input
      :disabled="disabled"
      :checked="value"
      v-on:input="$emit('input', $event.target.checked)"
      type="checkbox"
    />
    <span class="slider round"></span>
  </label>
</template>

<script>
export default {
  name: "SlideSwitch",
  props: {
    disabled: { type: Boolean, default: false },
    value: { type: Boolean, required: true },
  },
};
</script>

<style scoped>
.switch {
  position: relative;
  display: inline-block;
  width: 60px;
  height: 30px;
}
.switch input {
  opacity: 0;
  width: 0;
  height: 0;
}
.slider {
  position: absolute;
  cursor: pointer;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: #ccc;
  -webkit-transition: 0.4s;
  transition: 0.4s;
}
.slider:before {
  position: absolute;
  content: "";
  height: 20px;
  width: 20px;
  left: 7px;
  bottom: 5px;
  background-color: white;
  -webkit-transition: 0.15s;
  transition: 0.15s;
}
input:checked + .slider {
  background-color: #ef5350;
}
input:disabled + .slider {
  background-color: #ef5350;
}
input:focus + .slider {
  box-shadow: 0 0 1px #ef5350;
}
input:checked + .slider:before {
  -webkit-transform: translateX(26px);
  -ms-transform: translateX(26px);
  transform: translateX(26px);
}
.slider.round {
  border-radius: 20px;
}
.slider.round:before {
  border-radius: 50%;
}
</style>