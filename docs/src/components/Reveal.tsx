import React, { useEffect, useRef } from "react";
import { motion, useAnimation, useInView } from "framer-motion";

interface RevealProps {
    children: React.ReactNode;
    width?: "fit-content" | "100%";
}

export const Reveal: React.FC<RevealProps> = ({ children, width = "fit-content" }) => {
    const ref = useRef<HTMLDivElement>(null);
    const isInView = useInView(ref, { once: false }); // Animation triggers every time the component is in view
    const controls = useAnimation();

    useEffect(() => {
        if (isInView) {
            controls.start("visible"); // Start animation when in view
        } else {
            controls.start("hidden"); // Reset animation when out of view
        }
    }, [isInView, controls]);

    return (
        <motion.div
            ref={ref} // Attach ref to detect visibility
            style={{
                position: "relative",
                width,
                overflow: "hidden",
            }}
            initial="hidden" // Initial animation state
            animate={controls} // Controlled by visibility
            variants={{
                hidden: { opacity: 0, y: 75 },
                visible: { opacity: 1, y: 0 },
            }}
            transition={{ duration: 0.5 }} // Smooth animation
        >
            {children}
        </motion.div>
    );
};
