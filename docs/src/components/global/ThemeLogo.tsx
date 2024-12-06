import styles from '@site/src/css/style';
import { FlockMTLHorizontal, FlockMTLHorizontalDark } from '@site/static';

const ThemeLogo = () => {
    return (
        <a href='/flockmtl-website/' className={styles.animate}>
            <FlockMTLHorizontalDark
                width={150}
                height={50}
                className='hidden dark:block'
            />
            <FlockMTLHorizontal
                width={150}
                height={50}
                className='block dark:hidden'
            />
        </a>
    );
};

export default ThemeLogo;
