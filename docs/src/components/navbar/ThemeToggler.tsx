import styles from '@site/src/css/style';
import { useState, useEffect } from 'react';
import { MdOutlineDarkMode, MdOutlineLightMode } from 'react-icons/md';

const ThemeToggler = () => {
    const [theme, setTheme] = useState(() => {
        // Default to user's system preference or light theme
        return localStorage.getItem('theme') || (window.matchMedia('(data-theme: dark)').matches ? 'dark' : 'light');
    });

    useEffect(() => {
        // Apply the theme to the HTML element
        document.documentElement.setAttribute('data-theme', theme);
        localStorage.setItem('theme', theme); // Persist theme preference
    }, [theme]);

    const toggleTheme = () => {
        setTheme((prevTheme) => (prevTheme === 'dark' ? 'light' : 'dark'));
    };

    return (

        <div onClick={toggleTheme} className={`rounded-full p-2 text-2xl bg-[#FF9128] text-white flex cursor-pointer ${styles.animate}`}>
            {theme === 'dark' ?
                <MdOutlineDarkMode/>
                :
                <MdOutlineLightMode />
            }
        </div>
    );
};

export default ThemeToggler;