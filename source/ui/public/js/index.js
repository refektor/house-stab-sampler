import * as Juce from "./juce/index.js";

const NUM_PRESETS = 38;
let firstLoad = true;

document.addEventListener('DOMContentLoaded', (event) => {
    if (firstLoad) {
        function setAttack(value) {
            const attackValue = value/100 + 0.1;
            window.__JUCE__.backend.emitEvent('setAttack', {attack: attackValue});
        }

        function setDecay(value) {
            const decayValue = value/100 + 0.1;
            window.__JUCE__.backend.emitEvent('setDecay', {decay: decayValue});
        }

        function setSustain(value) {
            const sustainValue = value/100;
            window.__JUCE__.backend.emitEvent('setSustain', {sustain: sustainValue});
        }

        function setRelease(value) {
            const releaseValue = value/100 + 0.1;
            window.__JUCE__.backend.emitEvent('setRelease', {release: releaseValue});
        }

        function setReverbMix(value) {
            const mixValue = value/100;
            window.__JUCE__.backend.emitEvent('setReverbMix', {mix: mixValue});
        }

        function setDelayMix(value) {
            const mixValue = value/100;
            window.__JUCE__.backend.emitEvent('setDelayMix', {mix: mixValue});
        }

        function setChorusMix(value) {
            const mixValue = value/100;
            window.__JUCE__.backend.emitEvent('setChorusMix', {mix: mixValue});
        }

        function setSaturationDrive(value) {
            const driveValue = (value*3/100) + 1;
            window.__JUCE__.backend.emitEvent('setSaturationDrive', {drive: driveValue});
        }

        const dropdown = document.getElementById('presets');

        for (let i = 1; i <= NUM_PRESETS; i++) {
            const option = document.createElement('option');
            option.text = `Preset ${i}`;
            dropdown.appendChild(option);
        }
        
        dropdown.addEventListener('change', (event) => {
            const selectedIndex = event.target.selectedIndex;
            window.__JUCE__.backend.emitEvent('presetSelectionChanged', {presetIndex: selectedIndex});
        });

        document.querySelectorAll('.knob').forEach(knob => {
            let isDragging = false;
            let startY = 0;
            let currentRotation = (knob.id === "sustain") ? 135 : -135; // Initial rotation angle (start position)
            const maxRotation = 135; // Maximum rotation angle (end position)
        
            // Apply initial rotation
            knob.style.transform = `rotate(${currentRotation}deg)`;
        
            knob.addEventListener('mousedown', (e) => {
                isDragging = true;
                startY = e.clientY;
                knob.classList.add('active'); // Add active class on mousedown
                knob.style.transition = 'none'; // Disable transition during drag
            });
        
            document.addEventListener('mousemove', (e) => {
                if (isDragging) {
                    const deltaY = startY - e.clientY;
                    currentRotation += deltaY;
                    currentRotation = Math.max(-135, Math.min(maxRotation, currentRotation)); // Limit rotation between -135 and 135 degrees
                    knob.style.transform = `rotate(${currentRotation}deg)`;
                    startY = e.clientY;
        
                    // Capture the value (0 to 100 based on rotation)
                    const value = Math.round(((currentRotation + 135) / 270) * 100);
                    console.log(`Knob ${knob.id} value: ${value}`);

                    if (knob.id === 'attack') {
                        setAttack(value);
                    } else if (knob.id === 'decay') {
                        setDecay(value);
                    } else if (knob.id === 'sustain') {
                        setSustain(value);
                    } else if (knob.id === 'release') {
                        setRelease(value);  
                    } else if (knob.id === 'reverb') {
                        setReverbMix(value);
                    } else if (knob.id === 'delay') {
                        setDelayMix(value);
                    } else if (knob.id === 'chorus') {
                        setChorusMix(value);    
                    } else if (knob.id === 'drive') {
                        setSaturationDrive(value);
                    }   
                }
            });
        
            document.addEventListener('mouseup', () => {
                isDragging = false;
                knob.style.transition = ''; // Re-enable transition after drag
                knob.classList.remove('active'); // Add active class on mousedown
            });
        });


        // needle animation
        const canvas = document.getElementById('needleCanvas');
        const ctx = canvas.getContext('2d');

        canvas.width = canvas.offsetWidth;
        canvas.height = canvas.offsetHeight;

        const centerX = canvas.width / 2;
        const centerY = canvas.height * 0.9; // Position the pivot slightly lower
        const radius = canvas.width * 0.4; // Adjust radius for needle length

        // Animate the needle
        let currentValue = 0;

        function drawNeedle(value) {
            ctx.clearRect(0, 0, canvas.width, canvas.height);

            // Convert value to angle (invert angles for upward orientation)
            const startAngle = Math.PI * 0.90; // Leftmost position (-20 dB)
            const endAngle = Math.PI * 0.1; // Rightmost position (+3 dB)
            const angle = startAngle - value * (startAngle - endAngle); // Inverted logic

            // Draw the needle
            ctx.beginPath();
            ctx.moveTo(centerX, centerY);
            const needleX = centerX + radius * Math.cos(angle);
            const needleY = centerY - radius * Math.sin(angle); // Subtract for upward orientation
            ctx.lineTo(needleX, needleY);
            ctx.lineWidth = 3;
            ctx.strokeStyle = "#c82424";
            ctx.stroke();

            // Draw the needle pivot
            ctx.beginPath();
            ctx.arc(centerX, centerY, 8, 0, Math.PI * 2);
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

        window.addPresetOptions = function(numPresets) {
            const dropdown = document.getElementById('presets');
        
            for (let i = 1; i <= numPresets; i++) {
                const option = document.createElement('option');
                option.text = `Preset ${i}`;
                dropdown.appendChild(option);
            }
        };
        
        window.updateNeedle = function(rmsLevel) {
            currentValue = rmsLevel;
            currentValue = Math.max(0, Math.min(1, currentValue)); // Clamp value between 0 and 1
            drawNeedle(currentValue);
        };

        firstLoad = false;
    }
});




