import BrowserOnly from '@docusaurus/BrowserOnly';
import React from 'react';
import { translate } from '@docusaurus/Translate';
import { PageMetadata } from '@docusaurus/theme-common';
import { Navbar } from '@site/src/components';
import styles from '@site/src/css/style';
import { Reveal } from '@site/src/components/Reveal';

export default function Index(): JSX.Element {
  const title = translate({
    id: 'theme.NotFound.title',
    message: 'Page Not Found',
  });
  return (
    <>
      <PageMetadata title={title} />
      <BrowserOnly>
        {() => (
        <div className="bg-primary w-full overflow-hidden">
          <div className={`${styles.paddingX} ${styles.flexCenter}`}>
            <div className={`${styles.boxWidth} flex justify-center`}>
              <Navbar />
            </div>
          </div>
          <div className={`bg-primary ${styles.flexStart}`}>
            <div className={`${styles.boxWidth}`}>
              <Reveal width="100%">
                <div className='text-center'>
                  <h1>{title}</h1>
                  <p className=''>The page you are looking for does not exist.</p>
                  <a href="/flock-website/">Go back to the home page</a>
                </div>
              </Reveal>
            </div>
          </div>
        </div>
        )}
      </BrowserOnly>
    </>
  )
}
