<template>
  <div class="slidecontainer">
    <input
      :value="value"
      @input="$emit('input', parseInt($event.target.value))"
      :disabled="disabled"
      type="range"
      :min="min"
      :max="max"
      class="slider"
    />
    <span>{{ value }}</span>
  </div>
</template>

<script>
export default {
  name: "Slider",
  props: {
    value: { required: true },
    min: { type: Number, required: true },
    max: { type: Number, required: true },
    disabled: { type: Boolean, default: false },
  },
};
</script>

<style scoped>
.slidecontainer {
  width: 100%;
  display: flex;
}

.slider {
  flex-grow: 1;
  -webkit-appearance: none;
  /*width: 80%;*/
  height: 15px;
  border-radius: 5px;
  background: #d3d3d3;
  outline: none;
  opacity: 0.7;
  -webkit-transition: 0.2s;
  transition: opacity 0.2s;
}

.slider:hover {
  opacity: 1;
}

.slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 25px;
  height: 25px;
  border-radius: 50%;
  background: #ef5350;
  cursor: pointer;
}

.slider::-moz-range-thumb {
  width: 25px;
  height: 25px;
  border-radius: 50%;
  background: #ef5350;
  cursor: pointer;
}
</style>