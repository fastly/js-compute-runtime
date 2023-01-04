import React from 'react';
import clsx from 'clsx';
import styles from './styles.module.css';

const FeatureList = [
  {
    title: 'Service Worker API',
    description: (
      <>
      Applications are written using the same Service Worker API that is used for browsers. Aiming to make it simpler to re-use code across different environments.
      </>
    ),
  },
  {
    title: 'Powered by SpiderMonkey',
    description: (
      <>
      Built using the same secure, well-tested, standards-compliant JavaScript engine which powers Mozilla's Firefox.
      </>
    ),
  },
  {
    title: 'Open source',
    description: (
      <>
        Want to see behind the curtains?<br/><a href="https://github.com/fastly/js-compute-runtime">View the GitHub Repository</a>.<br/>Your contributions are welcomed and appreciated.
      </>
    ),
  },
];

function Feature({Svg, title, description}) {
  return (
    <div className={clsx('col col--4')}>
      <div className="text--center">
      </div>
      <div className="text--center padding-horiz--md">
        <h3>{title}</h3>
        <p>{description}</p>
      </div>
    </div>
  );
}

export default function HomepageFeatures() {
  return (
    <section className={styles.features}>
      <div className="container">
        <div className="row">
          {FeatureList.map((props, idx) => (
            <Feature key={idx} {...props} />
          ))}
        </div>
      </div>
    </section>
  );
}
