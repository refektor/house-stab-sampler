import * as Juce from "./juce/juce_index.js";

const NUM_PRESETS = 38;
let firstLoad = true;

document.addEventListener("DOMContentLoaded", (event) => {

    let currentRotations = {};

  if (firstLoad) {
    // Set up preset selection
    const presetState = Juce.getComboBoxState("presetIndex");
    presetState.valueChangedEvent.addListener(() => {
      const presetIndex = presetState.getChoiceIndex();
      const dropdown = document.getElementById("presets");
      if (dropdown && dropdown.selectedIndex !== presetIndex) {
        dropdown.selectedIndex = presetIndex;
      }
    });

    presetState.propertiesChangedEvent.addListener(() => {
      const dropdown = document.getElementById("presets");
      dropdown.innerHTML = "";
      presetState.properties.choices.forEach((choice) => {
        dropdown.innerHTML += `<option value=\"${choice}\">${choice}</option>`;
      });
    });

    const reverbSliderState = Juce.getSliderState("reverbMix");
    reverbSliderState.valueChangedEvent.addListener(() => {
        const value = reverbSliderState.getScaledValue();
        const rotation = (value) * 270 - 135;
        const knob = document.getElementById("reverb");
        knob.style.transform = `rotate(${rotation}deg)`;
        currentRotations[knob.id] = rotation;
    });

    const delaySliderState = Juce.getSliderState("delayMix");
    delaySliderState.valueChangedEvent.addListener(() => {
        const value = delaySliderState.getScaledValue();
        const rotation = (value) * 270 - 135;
        const knob = document.getElementById("delay");
        knob.style.transform = `rotate(${rotation}deg)`;
        currentRotations[knob.id] = rotation;
    });

    const chorusSliderState = Juce.getSliderState("chorusMix");
    chorusSliderState.valueChangedEvent.addListener(() => {
        const value = chorusSliderState.getScaledValue();
        const rotation = (value) * 270 - 135;
        const knob = document.getElementById("chorus");
        knob.style.transform = `rotate(${rotation}deg)`;
        currentRotations[knob.id] = rotation;
    });

    const driveSliderState = Juce.getSliderState("saturationDrive");
    driveSliderState.valueChangedEvent.addListener(() => {
        const value = driveSliderState.getScaledValue();
        const rotation = (value) * 270 - 135;
        const knob = document.getElementById("drive");
        knob.style.transform = `rotate(${rotation}deg)`;
        currentRotations[knob.id] = rotation;
    });

    const attackSliderState = Juce.getSliderState("attack");
    attackSliderState.valueChangedEvent.addListener(() => {
        const value = attackSliderState.getScaledValue();
        const rotation = (value) * 270 - 135;
        const knob = document.getElementById("attack");
        knob.style.transform = `rotate(${rotation}deg)`;
        currentRotations[knob.id] = rotation;
    });

    const decaySliderState = Juce.getSliderState("decay");
    decaySliderState.valueChangedEvent.addListener(() => {
        const value = decaySliderState.getScaledValue();
        const rotation = (value) * 270 - 135;
        const knob = document.getElementById("decay");
        knob.style.transform = `rotate(${rotation}deg)`;
        currentRotations[knob.id] = rotation;
    });

    const sustainSliderState = Juce.getSliderState("sustain");
    sustainSliderState.valueChangedEvent.addListener(() => {
        const value = sustainSliderState.getScaledValue();
        const rotation = value * 270 - 135;
        const knob = document.getElementById("sustain");
        knob.style.transform = `rotate(${rotation}deg)`;
        currentRotations[knob.id] = rotation;
    });

    const releaseSliderState = Juce.getSliderState("release"); 
    releaseSliderState.valueChangedEvent.addListener(() => {
        const value = releaseSliderState.getScaledValue();
        const rotation = (value) * 270 - 135;
        const knob = document.getElementById("release");
        knob.style.transform = `rotate(${rotation}deg)`;
        currentRotations[knob.id] = rotation;
    });

    function setAttack(value) {
      const attackValue = value / 100;
      //window.__JUCE__.backend.emitEvent("setAttack", { attack: attackValue });
      attackSliderState.setNormalisedValue(attackValue);
    }

    function setDecay(value) {
        const decayValue = value / 100;
        //window.__JUCE__.backend.emitEvent("setDecay", { decay: decayValue });
        decaySliderState.setNormalisedValue(decayValue);
    }

    function setSustain(value) {
        const sustainValue = value / 100;
        // window.__JUCE__.backend.emitEvent("setSustain", {
        //     sustain: sustainValue,
        // });
        sustainSliderState.setNormalisedValue(sustainValue);
    }

    function setRelease(value) {
        const releaseValue = value / 100;
        // window.__JUCE__.backend.emitEvent("setRelease", {
        // release: releaseValue,
        // });
        releaseSliderState.setNormalisedValue(releaseValue);
    }

    function setReverbMix(value) {
        const mixValue = value / 100;
        //window.__JUCE__.backend.emitEvent("setReverbMix", { mix: mixValue });
        reverbSliderState.setNormalisedValue(mixValue);
    }

    function setDelayMix(value) {
        const mixValue = value / 100;
        //window.__JUCE__.backend.emitEvent("setDelayMix", { mix: mixValue });
        delaySliderState.setNormalisedValue(mixValue);
    }

    function setChorusMix(value) {
        const mixValue = value / 100;
        //window.__JUCE__.backend.emitEvent("setChorusMix", { mix: mixValue });
        chorusSliderState.setNormalisedValue(mixValue);
    }

    function setSaturationDrive(value) {
        const driveValue = (value) / 100;
        // window.__JUCE__.backend.emitEvent("setSaturationDrive", {
        //     drive: driveValue,
        // });
        driveSliderState.setNormalisedValue(driveValue);
    }

    // Filter & Output Knobs need to be connected to the backend
    function setLowCut(value) {
      const lowCutValue = value / 100;
      window.__JUCE__.backend.emitEvent("setLowCut", { lowCut: lowCutValue });
    }

    function setHiCut(value) {
      const hiCutValue = value / 100;
      window.__JUCE__.backend.emitEvent("setHiCut", { hiCut: hiCutValue });
    }

    function setOutputGain(value) {
      const outputGainValue = value / 100;
      window.__JUCE__.backend.emitEvent("setOutputGain", {
        outputGain: outputGainValue,
      });
    }

    const dropdown = document.getElementById("presets");

    for (let i = 1; i <= NUM_PRESETS; i++) {
      const option = document.createElement("option");
      option.text = `Preset ${i}`;
      dropdown.appendChild(option);
    }

    dropdown.addEventListener("change", (event) => {
      const selectedIndex = event.target.selectedIndex;
      // window.__JUCE__.backend.emitEvent("presetSelectionChanged", {
      //   presetIndex: selectedIndex,
      // });
      presetState.setChoiceIndex(selectedIndex);
    });

    document.querySelectorAll(".knob, .output-knob").forEach((knob) => {
      let isDragging = false;
      let startY = 0;

      let currentRotation = 0;
      if (currentRotations[knob.id]) {
        currentRotation = currentRotations[knob.id];
        } else {
            currentRotation =  knob.id === "sustain" || knob.id === "hicut"
            ? 135
            : knob.id === "gain"
            ? 0
            : -135; // Initial rotation angle (start position)
            currentRotations[knob.id] = currentRotation;
        }
        

      const maxRotation = 135; // Maximum rotation angle (end position)

      // Apply initial rotation
      knob.style.transform = `rotate(${currentRotation}deg)`;

      knob.addEventListener("mousedown", (e) => {
        isDragging = true;
        startY = e.clientY;
        knob.classList.add("active"); // Add active class on mousedown
        knob.style.transition = "none"; // Disable transition during drag
      });

      document.addEventListener("mousemove", (e) => {
        if (isDragging) {
          const deltaY = startY - e.clientY;
          console.log(e.clientY);
          currentRotations[knob.id] += deltaY;
          currentRotations[knob.id] = Math.max(
            -135,
            Math.min(maxRotation, currentRotations[knob.id])
          ); // Limit rotation between -135 and 135 degrees
          knob.style.transform = `rotate(${currentRotations[knob.id]}deg)`;
          startY = e.clientY;

          // Capture the value (0 to 100 based on rotation)
          const value = Math.round(((currentRotations[knob.id] + 135) / 270) * 100);
          console.log(`Knob ${knob.id} value: ${value}`);

          if (knob.id === "attack") {
            setAttack(value);
          } else if (knob.id === "decay") {
            setDecay(value);
          } else if (knob.id === "sustain") {
            setSustain(value);
          } else if (knob.id === "release") {
            setRelease(value);
          } else if (knob.id === "reverb") {
            setReverbMix(value);
          } else if (knob.id === "delay") {
            setDelayMix(value);
          } else if (knob.id === "chorus") {
            setChorusMix(value);
          } else if (knob.id === "drive") {
            setSaturationDrive(value);
            // New filter and gain knobs
          } else if (knob.id === "lowcut") {
            setLowCut(value);
          } else if (knob.id === "hicut") {
            setHiCut(value);
          } else if (knob.id === "gain") {
            setOutputGain(value);
          }
        }
      });

      document.addEventListener("mouseup", () => {
        isDragging = false;
        knob.style.transition = ""; // Re-enable transition after drag
        knob.classList.remove("active"); // Add active class on mousedown
      });
    });

    // needle animation
    const canvas = document.getElementById("needleCanvas");
    const ctx = canvas.getContext("2d");

    // Get the device pixel ratio
    const dpr = window.devicePixelRatio || 1;

    // Get the canvas's styled dimensions
    const rect = canvas.getBoundingClientRect();

    // Set the canvas's internal dimensions scaled by DPR
    canvas.width = rect.width * dpr;
    canvas.height = rect.height * dpr;

    // Scale the canvas context
    ctx.scale(dpr, dpr);

    // Set the canvas's styled dimensions
    // canvas.style.width = `${rect.width}px`;
    // canvas.style.height = `${rect.height}px`;

    const centerX = rect.width / 2;
    const centerY = rect.height * 0.9; // Position the pivot slightly lower
    const radius = rect.width * 0.47; // Adjust radius for needle length

    // Animate the needle
    let currentValue = 0;

    function drawNeedle(value) {
      ctx.clearRect(0, 0, canvas.width, canvas.height);

      // Convert value to angle (invert angles for upward orientation)
      const startAngle = Math.PI * 0.8; // Leftmost position (-20 dB)
      const endAngle = Math.PI * 0.2; // Rightmost position (+3 dB)
      const angle = startAngle - value * (startAngle - endAngle); // Inverted logic

      // Draw the needle
      ctx.beginPath();
      ctx.moveTo(centerX, centerY);
      const needleX = centerX + radius * Math.cos(angle);
      const needleY = centerY - radius * Math.sin(angle); // Subtract for upward orientation
      ctx.lineTo(needleX, needleY);
      ctx.lineWidth = 2;
      ctx.strokeStyle = "#c82424";
      ctx.stroke();

      // Draw the needle pivot
      ctx.beginPath();
      ctx.arc(centerX, centerY, 4, 0, Math.PI * 2);
      ctx.fillStyle = "#01080d";
      ctx.fill();
    }

    function updateMeter() {
      currentValue = 0;
      drawNeedle(currentValue);
      //requestAnimationFrame(updateMeter);
    }

    // Start animation
    updateMeter();

    

    window.addPresetOptions = function (numPresets) {
      const dropdown = document.getElementById("presets");

      for (let i = 1; i <= numPresets; i++) {
        const option = document.createElement("option");
        option.text = `Preset ${i}`;
        dropdown.appendChild(option);
      }
    };

    window.updateNeedle = function (volumeLevel) {
      currentValue = volumeLevel;
      currentValue = Math.max(0, Math.min(1, currentValue)); // Clamp value between 0 and 1
      drawNeedle(currentValue);
    };

    firstLoad = false;
  }
});


