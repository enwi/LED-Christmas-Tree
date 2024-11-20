<template>
  <input
    type="number"
    :disabled="disabled"
    @input="$emit('input', parseInt($event.target.value))"
    :value="value"
  />
</template>

<script>
export default {
  name: "NumberInput",
  props: {
    disabled: { type: Boolean, default: false },
    value: { required: true },
  },
};
</script>

<style scoped>
input:disabled {
  opacity: 0.25;
}
input {
  padding: 8px;
  font-size: 0.9rem;
  box-sizing: border-box;
  width: 100%;
  border: 1px solid gray;
  border-radius: 5px;
  -webkit-appearance: none;
}
input {
  -webkit-appearance: menulist;
}
</style>