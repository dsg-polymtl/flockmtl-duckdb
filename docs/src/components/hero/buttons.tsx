import { Reveal } from "@site/src/components/Reveal";
import styles from "@site/src/css/style";

const Button = ({ children, className, href }: { children: React.ReactNode; className?: string, href?: string }) => {
    return (
        <Reveal>
            <a href={href} className={`${className} px-10 py-4 font-bold rounded-full flex items-center gap-2 ${styles.animate}`}>
                {children}
            </a>
        </Reveal>
    );
}

export default Button;