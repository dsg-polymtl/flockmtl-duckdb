import { Reveal } from "@site/src/components/Reveal";
import styles from "@site/src/css/style";

const Button = ({ href, children, className }: { href?: string, children: React.ReactNode; className?: string }) => {
    return (
        <Reveal>
            <a href={href} className={`${className} no-underline px-10 py-4 font-bold rounded-full flex items-center gap-2  ${styles.animate}`}>
                {children}
            </a>
        </Reveal>
    );
}

export default Button;