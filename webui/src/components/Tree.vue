<template>
  <div class="tree-container" :style="tree_style">
    <div class="tree">
      <div class="tree-triangle1"></div>
      <div class="tree-triangle2"></div>
      <div class="tree-triangle3"></div>
      <div class="trunk"></div>
    </div>
  </div>
</template>

<script>
export default {
  name: "Tree",
  props: {
    value: { type: Boolean },
  },
  computed: {
    tree_style() {
      return {
        filter: "grayscale(" + (this.value ? "0" : "1") + ")",
      };
    },
  },
};
</script>

<style scoped>
.tree-container {
  position: relative;
  width: 200px;
  height: 100%;
  z-index: -2;
}
.tree-triangle3 {
  position: absolute;
  width: 80%;
  height: 45%;
  bottom: 20%;
  left: 10%;
  background: #034f33;
  clip-path: polygon(50% 0%, 0% 100%, 100% 100%);
}
.tree-triangle2 {
  position: absolute;
  width: 70%;
  height: 43%;
  bottom: 35%;
  left: 15%;
  clip-path: polygon(50% 0%, 0% 100%, 100% 100%);
  background: #046944;
  z-index: 2;
}
.tree-triangle1 {
  position: absolute;
  width: 60%;
  height: 40%;
  bottom: 50%;
  left: 20%;
  background: #038052;
  clip-path: polygon(50% 0%, 0% 100%, 100% 100%);
  z-index: 3;
}
.trunk {
  position: absolute;
  width: 10%;
  height: 16%;
  background: #66503e;
  z-index: -1;
  bottom: 5%;
  left: 50%;
  transform: translateX(-50%);
}
</style>